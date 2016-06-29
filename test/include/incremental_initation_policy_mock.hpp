#ifndef DIPLOMA_INCREMENTAL_INITATION_POLICY_MOCK_HPP
#define DIPLOMA_INCREMENTAL_INITATION_POLICY_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/collectors/initation_policy.hpp>

class incremental_initation_policy_mock : public precisegc::details::collectors::incremental_initation_policy
{
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::gc_state gc_stat;
    typedef precisegc::details::initation_point_type initation_point_type;
public:
    MOCK_CONST_METHOD2(check, gc_phase(const gc_stat&, initation_point_type));
    MOCK_METHOD2(update, void(const gc_stat&, initation_point_type));
};

#endif //DIPLOMA_INCREMENTAL_INITATION_POLICY_MOCK_HPP
