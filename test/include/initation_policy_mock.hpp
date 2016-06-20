#ifndef DIPLOMA_INITATION_POLICY_MOCK_HPP
#define DIPLOMA_INITATION_POLICY_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/initation_policy.hpp>

class initation_policy_mock : public precisegc::details::initation_policy
{
    typedef precisegc::details::gc_stat gc_stat;
    typedef precisegc::details::initation_point_type initation_point_type;
public:
    MOCK_CONST_METHOD2(check, bool(const gc_stat&, initation_point_type));
    MOCK_METHOD2(update, void(const gc_stat&, initation_point_type));
};

#endif //DIPLOMA_INITATION_POLICY_MOCK_HPP
