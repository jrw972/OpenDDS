// -*- C++ -*-
// ============================================================================
/**
 *  @file   subscriber.cpp
 *
 *
 *
 */
// ============================================================================


#include "DataReaderListener.h"
#include "MessengerTypeSupportImpl.h"
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/SubscriberImpl.h>
#include "dds/DCPS/StaticIncludes.h"

#include <ace/streams.h>
#include "tests/Utils/ExceptionStreams.h"
#include <tests/Utils/DistributedConditionSet.h>
#include <tests/Utils/StatusMatching.h>
#include "ace/Get_Opt.h"
#include "ace/OS_NS_unistd.h"

using namespace Messenger;
using namespace std;

long num_expected_dispose = 0;
long num_expected_unregister = 0;
long num_expected_data = 10;

int
parse_args (int argc, ACE_TCHAR *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, ACE_TEXT("du"));
  int c;

  while ((c = get_opts ()) != -1)
  {
    switch (c)
    {
    case 'd':
      num_expected_dispose = 1;
      break;
    case 'u':
      num_expected_unregister = 1;
      break;
    case '?':
    default:
      ACE_ERROR_RETURN ((LM_ERROR,
        "usage:  %s "
        "-d -u"
        "\n"
        "-d for dispose notification test and -u for unregister notification test\n",
        argv [0]),
        -1);
    }
  }
  // Indicates successful parsing of the command line
  return 0;
}


int ACE_TMAIN (int argc, ACE_TCHAR *argv[])
{
  try
    {
      DistributedConditionSet_rch dcs =
        OpenDDS::DCPS::make_rch<FileBasedDistributedConditionSet>();

      DDS::DomainParticipantFactory_var dpf;
      DDS::DomainParticipant_var participant;

      dpf = TheParticipantFactoryWithArgs(argc, argv);
      participant = dpf->create_participant(111,
                                            PARTICIPANT_QOS_DEFAULT,
                                            DDS::DomainParticipantListener::_nil(),
                                            ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (participant.in ())) {
        cerr << "create_participant failed." << endl;
        return 1 ;
      }

      if (parse_args (argc, argv) == -1) {
        return -1;
      }

      MessageTypeSupport_var mts = new MessageTypeSupportImpl();

      if (DDS::RETCODE_OK != mts->register_type(participant.in (), "")) {
          cerr << "Failed to register the MessageTypeTypeSupport." << endl;
          exit(1);
        }

      CORBA::String_var type_name = mts->get_type_name ();

      DDS::Topic_var topic = participant->create_topic("Movie Discussion List",
                                                       type_name.in (),
                                                       TOPIC_QOS_DEFAULT,
                                                       DDS::TopicListener::_nil(),
                                                       ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (topic.in ())) {
        cerr << "Failed to create_topic." << endl;
        exit(1);
      }

      // Create the subscriber
      DDS::Subscriber_var sub =
        participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT,
                                       DDS::SubscriberListener::_nil(),
                                       ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (sub.in ())) {
        cerr << "Failed to create_subscriber." << endl;
        exit(1);
      }

      cerr << " expected " <<  num_expected_data << "/" << num_expected_dispose << "/" << num_expected_unregister <<endl;

      // activate the listener
      DDS::DataReaderListener_var listener = new DataReaderListenerImpl(dcs, num_expected_data);;
      DataReaderListenerImpl &listener_servant =
        *dynamic_cast<DataReaderListenerImpl*>(listener.in());

      if (CORBA::is_nil (listener.in ())) {
        cerr << "listener is nil." << endl;
        exit(1);
      }

      // Create the Datareaders
      DDS::DataReaderQos dr_qos;
      sub->get_default_datareader_qos(dr_qos);
      dr_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;

      DDS::DataReader_var dr = sub->create_datareader(topic.in (),
                                                      dr_qos,
                                                      listener.in (),
                                                      ::OpenDDS::DCPS::DEFAULT_STATUS_MASK);
      if (CORBA::is_nil (dr.in ())) {
        cerr << "create_datareader failed." << endl;
        exit(1);
      }

      Utils::wait_match(dr, 1);
      dcs->post("sub", "ready");

      if (num_expected_dispose) {
        dcs->wait_for("sub", "sub", "dispose");
      }
      if (num_expected_unregister) {
        dcs->wait_for("sub", "sub", "unregister");
      }
      dcs->wait_for("sub", "sub", "data");

      listener_servant.stop ();

      if (!CORBA::is_nil (participant.in ())) {
        participant->delete_contained_entities();
      }
      if (!CORBA::is_nil (dpf.in ())) {
        dpf->delete_participant(participant.in ());
      }

      TheServiceParticipant->shutdown ();

      dcs->post("sub", "done");

      if (listener_servant.num_received_dispose () != num_expected_dispose)
      {
         cerr << "did not receive dispose sample as expected." << num_expected_dispose
           << "/" << listener_servant.num_received_dispose () << endl;
         return 1;
      }
      if (listener_servant.num_received_unregister () != num_expected_unregister)
      {
         cerr << "did not receive unregister sample as expected." << num_expected_unregister
           << "/" << listener_servant.num_received_unregister () << endl;
         return 1;
      }

    }
  catch (CORBA::Exception& e)
    {
      cerr << "SUB: Exception caught in main ():" << endl << e << endl;
      return 1;
    }

  return 0;
}
