/*
 *
 *
 * Distributed under the OpenDDS License.
 * See: http://www.opendds.org/license.html
 */

#include "ReceivedDataElementList.h"
#include "ace/OS_NS_sys_time.h"

OPENDDS_BEGIN_VERSIONED_NAMESPACE_DECL

ACE_INLINE
OpenDDS::DCPS::WeakRcHandle<OpenDDS::DCPS::DataReaderImpl>
OpenDDS::DCPS::InstanceState::data_reader() const
{
  return reader_;
}

ACE_INLINE
void
OpenDDS::DCPS::InstanceState::accessed(InstanceStateUpdateList& isul)
{
  //
  // Manage the view state due to data access here.
  //
  if (view_state_ & DDS::ANY_VIEW_STATE) {
    const CORBA::ULong previous_state = combined_state_i();
    view_state_ = DDS::NOT_NEW_VIEW_STATE;
    isul.add(handle_, previous_state, combined_state_i());
  }
}

ACE_INLINE
DDS::InstanceStateKind
OpenDDS::DCPS::InstanceState::instance_state() const
{
  return instance_state_;
}

ACE_INLINE
DDS::ViewStateKind
OpenDDS::DCPS::InstanceState::view_state() const
{
  return view_state_;
}

ACE_INLINE
bool OpenDDS::DCPS::InstanceState::match(DDS::ViewStateMask view, DDS::InstanceStateMask inst) const
{
  return (view_state_ & view) && (instance_state_ & inst);
}

ACE_INLINE
size_t OpenDDS::DCPS::InstanceState::disposed_generation_count() const
{
  return disposed_generation_count_;
}

ACE_INLINE
size_t OpenDDS::DCPS::InstanceState::no_writers_generation_count() const
{
  return no_writers_generation_count_;
}

ACE_INLINE
void
OpenDDS::DCPS::InstanceState::data_was_received(const PublicationId& writer_id,
                                                InstanceStateUpdateList& isul)
{
  cancel_release();

  //
  // Update the view state here, since only sample data received affects
  // this state value.  Then manage the data sample only transitions
  // here.  Let the lively() method manage the other transitions.
  //
  writers_.insert(writer_id);

  switch (view_state_) {
  case DDS::NEW_VIEW_STATE:
    break;

  case DDS::NOT_NEW_VIEW_STATE:
    if (instance_state_ & DDS::NOT_ALIVE_INSTANCE_STATE) {
      const CORBA::ULong previous_state = combined_state_i();
      view_state_ = DDS::NEW_VIEW_STATE;
      isul.add(handle_, previous_state, combined_state_i());
    }
    break;

  default:
    {
      const CORBA::ULong previous_state = combined_state_i();
      view_state_ = DDS::NEW_VIEW_STATE;
      isul.add(handle_, previous_state, combined_state_i());
    }
    break;
  }

  switch (instance_state_) {
  case DDS::NOT_ALIVE_DISPOSED_INSTANCE_STATE:
    ++disposed_generation_count_;
    break;

  case DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
    ++no_writers_generation_count_;
    break;

  default:
    break;
  }

  const CORBA::ULong previous_state = combined_state_i();
  instance_state_ = DDS::ALIVE_INSTANCE_STATE;
  isul.add(handle_, previous_state, combined_state_i());
}

ACE_INLINE
void
OpenDDS::DCPS::InstanceState::lively(const PublicationId& writer_id,
                                     InstanceStateUpdateList& isul)
{
  //
  // Manage transisitions in the instance state that do not require a
  // data sample, but merely the notion of liveliness.
  //
  writers_.insert(writer_id);

  if (instance_state_ == DDS::NOT_ALIVE_NO_WRITERS_INSTANCE_STATE) {
    cancel_release(); // cancel unregister

    ++no_writers_generation_count_;
    const CORBA::ULong previous_state = combined_state_i();
    instance_state_ = DDS::ALIVE_INSTANCE_STATE;
    isul.add(handle_, previous_state, combined_state_i());
  }
}

ACE_INLINE
bool
OpenDDS::DCPS::InstanceState::is_last (const PublicationId& pub)
{
  return writers_.size() == 1 && *writers_.begin() == pub;
}

ACE_INLINE
bool
OpenDDS::DCPS::InstanceState::no_writer () const
{
  return writers_.empty();
}

ACE_INLINE
const char* OpenDDS::DCPS::InstanceState::instance_state_string() const
{
  return instance_state_string(instance_state_);
}

OPENDDS_END_VERSIONED_NAMESPACE_DECL
