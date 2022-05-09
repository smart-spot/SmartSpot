#include <cascade/user_defined_logic_interface.hpp>
#include <iostream>

// Tensorflow packages
#include <mutex>
#include <vector>
#include <opencv2/opencv.hpp>
#include <tensorflow/c/c_api.h>
#include <tensorflow/c/eager/c_api.h>

// GRPC packages
#include <grpcpp/grpcpp.h>
#include "notification.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using notification::CarReply;
using notification::Notification;
using notificaton::CarNotification;

class NotificationClient
{
public:
  NotificationClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string CarAlert(const std::string &user, char[] carImage)
  {
    // Data we are sending to the server.
    CarNotification request;
    request.set_user_name(user);
    request.set_image(carImage);

    // Container for the data we expect from the server.
    CarReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->CarAlert(&context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
      return reply.message();
    }
    else
    {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<Greeter::Stub> stub_;
};

namespace derecho
{
  namespace cascade
  {

#define MY_UUID "48e60f7c-8500-11eb-8755-0242ac110002"
#define MY_DESC "Car Detector UDL that detects whether a car is in an image or not."

    std::string get_uuid()
    {
      return MY_UUID;
    }

    std::string get_description()
    {
      return MY_DESC;
    }

#define FILTER_THRESHOLD (0.9)
#define IMAGE_WIDTH (352)
#define IMAGE_HEIGHT (240)
#define FILTER_TENSOR_BUFFER_SIZE (IMAGE_WIDTH * IMAGE_HEIGHT * 3)
#define CONF_FILTER_MODEL "car-detector-model"

#define CHECK_STATUS(tfs)                     \
  if (TF_GetCode(tfs) != TF_OK)               \
  {                                           \
    std::runtime_error rerr(TF_Message(tfs)); \
    TF_DeleteStatus(tfs);                     \
    throw rerr;                               \
  }

    class ConsolePrinterOCDPO : public OffCriticalDataPathObserver // should be CarDetectorOCDPO
    {
      virtual void operator()(const node_id_t,
                              const std::string &key_string,
                              const uint32_t prefix_length,
                              persistent::version_t version,
                              const mutils::ByteRepresentable *const value_ptr,
                              const std::unordered_map<std::string, bool> &outputs,
                              ICascadeContext *ctxt,
                              uint32_t worker_id) override
      {
        auto *typed_ctxt = dynamic_cast<DefaultCascadeContextType *>(ctxt);
        // TENSORFLOW LOGIC
        /* step 1: load the model */
        static thread_local std::unique_ptr<TF_Graph, decltype(TF_DeleteGraph) &> graph = {TF_NewGraph(), TF_DeleteGraph};
        static thread_local std::unique_ptr<TF_SessionOptions, decltype(TF_DeleteSessionOptions) &> session_options{TF_NewSessionOptions(), TF_DeleteSessionOptions};
        static thread_local std::unique_ptr<TF_Buffer, decltype(TF_DeleteBuffer) &> run_options{TF_NewBufferFromString("", 0), TF_DeleteBuffer};
        static thread_local std::unique_ptr<TF_Buffer, decltype(TF_DeleteBuffer) &> meta_graph{TF_NewBuffer(), TF_DeleteBuffer};
        static thread_local auto session_deleter = [](TF_Session *sess)
        {
          auto tf_status = TF_NewStatus();
          TF_DeleteSession(sess, tf_status);
          CHECK_STATUS(tf_status);
          TF_DeleteStatus(tf_status);
        };
        static thread_local int tag_len = 1;
        static thread_local const char *tag = "serve";
        static thread_local std::unique_ptr<TF_Status, decltype(TF_DeleteStatus) &> tf_status{TF_NewStatus(), TF_DeleteStatus};
        static thread_local std::unique_ptr<TF_Session, decltype(session_deleter) &> session =
            {
                TF_LoadSessionFromSavedModel(session_options.get(), run_options.get(), CONF_FILTER_MODEL,
                                             &tag, tag_len, graph.get(), meta_graph.get(), tf_status.get()),
                session_deleter};
        CHECK_STATUS(tf_status.get());
        static thread_local TF_Output input_op = {
            .oper = TF_GraphOperationByName(graph.get(), "serving_default_conv2d_3_input"),
            .index = 0};
        if (!input_op.oper)
        {
          throw std::runtime_error("No operation with name 'serving_default_conv2d_3_input' is found.");
        }
        static thread_local TF_Output output_op = {
            .oper = TF_GraphOperationByName(graph.get(), "StatefulPartitionedCall"),
            .index = 0};
        if (!output_op.oper)
        {
          throw std::runtime_error("No operation with name 'StatefulPartitionedCall' is found.");
        }
        /* step 2: Load the image & convert to tensor */
        static thread_local std::vector<int64_t> shape = {1, IMAGE_WIDTH, IMAGE_HEIGHT, 3};
        const ObjectWithStringKey *tcss_value = reinterpret_cast<const ObjectWithStringKey *>(value_ptr);
        const FrameData *frame = reinterpret_cast<const FrameData *>(tcss_value->blob.bytes);
        dbg_default_trace("frame photoid is: " + std::to_string(frame->photo_id));
        dbg_default_trace("frame timestamp is: " + std::to_string(frame->timestamp));

        /* We do this copy because frame->data is const, which cannot be wrapped in a Tensor. */
        static thread_local float buf[FILTER_TENSOR_BUFFER_SIZE];
        std::memcpy(buf, frame->data, sizeof(frame->data));

        auto input_tensor = TF_NewTensor(
            TF_FLOAT, shape.data(), shape.size(), buf, sizeof(buf),
            [](void *, size_t, void *) { /*do nothing for stack memory,.*/ }, nullptr);
        assert(input_tensor);

        /* step 3: Predict */
        static thread_local auto output_vals = std::make_unique<TF_Tensor *[]>(1);
        TF_SessionRun(session.get(), nullptr, &input_op, &input_tensor, 1,
                      &output_op, output_vals.get(), 1,
                      nullptr, 0, nullptr, tf_status.get());
        CHECK_STATUS(tf_status.get());

        /* step 4: Send intermediate results to the next tier if image frame is meaningful */
        // prediction < 0.35 indicates strong possibility that the image frame captures full contour of the cow
        auto raw_data = TF_TensorData(*output_vals.get());
        float prediction = *static_cast<float *>(raw_data); // figure out how to configure output to what we need
        TF_DeleteTensor(input_tensor);

        // END TENSORFLOW LOGIC
        if (prediction < FILTER_THRESHOLD)
        {
          // GRPC LOGIC
          std::string target_str = "localhost:50051"; // replace with public ip address of the grpc server
          NotificationClient notifier(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
          std::string userName("tedi");
          char[] carImage = "tedi"; // replace with bytes of car image
          std::string reply = notifier.CarAlert(user, carImage);

          // INPUT CAR IMAGE INTO SECOND LAYER/OBJECT POOL (OPTIONAL - not needed for demo)
        }

        // UDL OUTPUT LOGIC (output car detected or not detected)
        std::cout << "[CAR DETECTOR UDL]: I(" << worker_id << ") received an object with key=" << key_string
                  << ", matching prefix=" << key_string.substr(0, prefix_length) << std::endl;
      }

      static std::shared_ptr<OffCriticalDataPathObserver> ocdpo_ptr;

    public:
      static void initialize()
      {
        if (!ocdpo_ptr)
        {
          ocdpo_ptr = std::make_shared<ConsolePrinterOCDPO>();
        }
      }
      static auto get()
      {
        return ocdpo_ptr;
      }
    };

    std::shared_ptr<OffCriticalDataPathObserver> ConsolePrinterOCDPO::ocdpo_ptr;

    void initialize(ICascadeContext *ctxt)
    {
      ConsolePrinterOCDPO::initialize();
    }

    std::shared_ptr<OffCriticalDataPathObserver> get_observer(
        ICascadeContext *, const nlohmann::json &)
    {
      return ConsolePrinterOCDPO::get();
    }

    void release(ICascadeContext *ctxt)
    {
      // nothing to release
      return;
    }

  } // namespace cascade
} // namespace derecho