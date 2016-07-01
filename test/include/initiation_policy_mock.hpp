#ifndef DIPLOMA_INITATION_POLICY_MOCK_HPP
#define DIPLOMA_INITATION_POLICY_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/initiation_policy.hpp>

class initiation_policy_mock : public precisegc::details::initiation_policy
{
    typedef precisegc::details::gc_state gc_stat;
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::initiation_point_type initation_point_type;
public:
    MOCK_CONST_METHOD2(check, gc_phase(initation_point_type, const gc_stat&));
    MOCK_METHOD1(update, void(const gc_stat&));
};

#endif //DIPLOMA_INITATION_POLICY_MOCK_HPP
