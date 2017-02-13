/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */
 #include <iostream>

#include "boilerplate/Boilerplate.h"
#include <dds/DCPS/Service_Participant.h>
#include <model/Sync.h>
#include <stdexcept>
#include "test/test.h"

#include "dds/DCPS/StaticIncludes.h"
#include "firmata.h"
#include "firmserial.h"
using namespace examples::boilerplate;

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  std::vector<firmata::PortInfo> ports = firmata::FirmSerial::listPorts();
  firmata::Firmata<firmata::Base, firmata::I2C>* f = NULL;
  firmata::FirmSerial* serialio;

  serialio = new firmata::FirmSerial("/dev/ttyACM0");
  while(serialio->available()) {
    f = new firmata::Firmata<firmata::Base, firmata::I2C>(serialio);
  }
  try {

    f->setSamplingInterval(100);

    f->reportAnalog(0, 1);
    f->reportAnalog(1, 1);

    f->reportDigital(0, 1);
    // Initialize DomainParticipantFactory, handling command line args
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv);

    // Create domain participant
    DDS::DomainParticipant_var participant = createParticipant(dpf);

    // Register type support and create topic
    DDS::Topic_var topic = createTopic(participant);

    // Create publisher
    DDS::Publisher_var publisher = createPublisher(participant);

    // Create data writer for the topic
    DDS::DataWriter_var writer = createDataWriter(publisher, topic);

    // Safely downcast data writer to type-specific data writer
    Template::MessageDataWriter_var msg_writer = narrow(writer);

    {

      while(1){
        // Block until Subscriber is available
        OpenDDS::Model::WriterSync ws(writer);

        f->parse();
        int a0 = f->analogRead("A0");
        float angle = (float)(a0 / 1023.0) * 180.0;
        // std::cout << "Analog Read: " << angle << std::endl;

        // Initialize samples
        Template::Message message;
        message.data = angle;

        // Override message count

        // Publish the message
        DDS::ReturnCode_t error = msg_writer->write(message,
                                                    DDS::HANDLE_NIL);
        if (error != DDS::RETCODE_OK) {
          ACE_ERROR((LM_ERROR,
                     ACE_TEXT("ERROR: %N:%l: main() -")
                     ACE_TEXT(" write returned %d!\n"), error));
        }

        // Prepare next sample
        // ++message.count;
        // ++message.subject_id;
    }

      // End of WriterSync scope - block until messages acknowledged
    }

    delete f;

    // Clean-up!
    cleanup(participant, dpf);

  } catch (const CORBA::Exception& e) {
    e._tao_print_exception("Exception caught in main():");
    return -1;
  } catch (std::runtime_error& err) {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: main() - %s\n"),
                      err.what()), -1);
  } catch (std::string& msg) {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: main() - %s\n"),
                      msg.c_str()), -1);
  }
    catch (firmata::IOException e) {
      ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: main() - %s\n"),
                        e.what()), -1);
  }
  catch (firmata::NotOpenException e) {
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: main() - %s\n"),
                      e.what()), -1);  }

  return 0;
}
