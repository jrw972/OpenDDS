#include <gtestWrapper.h>

#include <dds/DCPS/ReactorTask.h>

#include <dds/DCPS/TimeSource.h>

#include <ace/Select_Reactor.h>

#include <gtest/gtest.h>

using namespace OpenDDS::DCPS;

namespace {
  class MyTimeSource : public TimeSource {
  public:
    MOCK_CONST_METHOD0(monotonic_time_point_now, MonotonicTimePoint());
  };

  class MyReactor : public ACE_Reactor {
  public:
    MOCK_METHOD4(schedule_timer, long(ACE_Event_Handler*, const void*, const ACE_Time_Value&, const ACE_Time_Value&));
    MOCK_METHOD3(cancel_timer, int(long, const void**, int));
  };

  struct TestEventHandler : RcEventHandler {
    TestEventHandler()
      : calls_(0)
    {}

    int handle_timeout(const ACE_Time_Value&, const void*)
    {
      ++calls_;
      return 0;
    }

    int calls_;
  };
}


TEST(dds_DCPS_ReactorWrapper, test_schedule_sporadic)
{
  RcHandle<TestEventHandler> handler = make_rch<TestEventHandler>();
  MyReactor reactor;
  ReactorWrapper reactor_wrapper(&reactor);
  const TimeDuration td = TimeDuration::from_msec(10);

  EXPECT_CALL(reactor, schedule_timer(handler.get(), 0, td.value(), ACE_Time_Value::zero))
    .Times(1)
    .WillOnce(testing::Return(1));

  const ReactorWrapper::TimerId id = reactor_wrapper.schedule(*handler, 0, td);
  ASSERT_EQ(id, 1);
}

TEST(dds_DCPS_ReactorWrapper, test_schedule_periodic)
{
  RcHandle<TestEventHandler> handler = make_rch<TestEventHandler>();
  MyReactor reactor;
  ReactorWrapper reactor_wrapper(&reactor);
  const TimeDuration td = TimeDuration::from_msec(10);

  EXPECT_CALL(reactor, schedule_timer(handler.get(), 0, ACE_Time_Value::zero, td.value()))
    .Times(1)
    .WillOnce(testing::Return(1));

  const ReactorWrapper::TimerId id = reactor_wrapper.schedule(*handler, 0, TimeDuration(), td);
  ASSERT_EQ(id, 1);
}

TEST(dds_DCPS_ReactorWrapper, test_schedule_immediate)
{
  RcHandle<TestEventHandler> handler = make_rch<TestEventHandler>();
  MyReactor reactor;
  ReactorWrapper reactor_wrapper(&reactor);
  const TimeDuration td = TimeDuration::from_msec(0);

  EXPECT_CALL(reactor, schedule_timer(handler.get(), 0, td.value(), ACE_Time_Value::zero))
    .Times(1)
    .WillOnce(testing::Return(1));

  const ReactorWrapper::TimerId id = reactor_wrapper.schedule(*handler, 0, td);
  ASSERT_EQ(id, 1);
}

TEST(dds_DCPS_ReactorWrapper, test_schedule_negative)
{
  RcHandle<TestEventHandler> handler = make_rch<TestEventHandler>();
  MyReactor reactor;
  ReactorWrapper reactor_wrapper(&reactor);
  const TimeDuration td(-23);

  EXPECT_CALL(reactor, schedule_timer(handler.get(), 0, td.value(), ACE_Time_Value::zero))
    .Times(1)
    .WillOnce(testing::Return(1));

  const ReactorWrapper::TimerId id = reactor_wrapper.schedule(*handler, 0, td);
  ASSERT_EQ(id, 1);
}

TEST(dds_DCPS_ReactorWrapper, test_cancel)
{
  RcHandle<TestEventHandler> handler = make_rch<TestEventHandler>();
  MyReactor reactor;
  ReactorWrapper reactor_wrapper(&reactor);
  const TimeDuration td = TimeDuration::from_msec(10);

  EXPECT_CALL(reactor, schedule_timer(handler.get(), 0, td.value(), ACE_Time_Value::zero))
    .Times(1)
    .WillOnce(testing::Return(1));
  EXPECT_CALL(reactor, cancel_timer(1, 0, 1))
    .Times(1)
    .WillOnce(testing::Return(1));

  const ReactorWrapper::TimerId id = reactor_wrapper.schedule(*handler, 0, td);
  ASSERT_EQ(id, 1);

  reactor_wrapper.cancel(1);
}
