/*
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "DCPS/DdsDcps_pch.h" //Only the _pch include should start with DCPS/

#include "SubscriptionInstance.h"

#include "DataReaderImpl.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

namespace OpenDDS {
namespace DCPS {

SubscriptionInstance::SubscriptionInstance(DataReaderImpl* reader,
                                           const DDS::DataReaderQos& qos,
                                           DDS::InstanceHandle_t handle,
                                           bool owns_handle)
  : instance_handle_(handle)
  , owns_handle_(owns_handle)
  , deadline_timer_id_(-1)
  , instance_state_(make_rch<InstanceState>(reader, handle))
  , rcvd_samples_()
{
  switch (qos.destination_order.kind) {
  case DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
    rcvd_strategy_.reset(new ReceptionDataStrategy(rcvd_samples_));
    break;

  case DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
    rcvd_strategy_.reset(new SourceDataStrategy(rcvd_samples_));
    break;
  }

  if (!rcvd_strategy_) {
    ACE_ERROR((LM_ERROR,
                ACE_TEXT("(%P|%t) ERROR: SubscriptionInstance: ")
                ACE_TEXT("unable to allocate ReceiveDataStrategy!\n")));
  }
}

SubscriptionInstance::~SubscriptionInstance()
{
  purge_data();

  if (owns_handle_) {
    const RcHandle<DataReaderImpl> reader = instance_state_->data_reader().lock();
    if (reader) {
      reader->return_handle(instance_handle_);

#ifndef OPENDDS_NO_OWNERSHIP_KIND_EXCLUSIVE
      if (instance_state_->registered()) {
        DataReaderImpl::OwnershipManagerPtr om = reader->ownership_manager();
        if (om) om->remove_instance(rchandle_from(this));
      }
#endif

    }
  }
}

}
}
OPENDDS_END_VERSIONED_NAMESPACE_DECL
