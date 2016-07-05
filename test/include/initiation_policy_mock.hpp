#ifndef DIPLOMA_INITATION_POLICY_MOCK_HPP
#define DIPLOMA_INITATION_POLICY_MOCK_HPP

#include <gmock/gmock.h>


#include <libprecisegc/details/initiation_policy.hpp>

class initiation_policy_mock : public precisegc::details::initiation_policy
{
    typedef precisegc::details::gc_state gc_stat;
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::initiation_point_type initiation_point_type;
    typedef precisegc::details::initiation_point_data initiation_point_data;
public:
    MOCK_CONST_METHOD3(check, gc_phase(initiation_point_type, const initiation_point_data&, const gc_stat&));
    MOCK_METHOD1(update, void(const gc_stat&));
};

#endif //DIPLOMA_INITATION_POLICY_MOCK_HPP
