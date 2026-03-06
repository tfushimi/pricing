#pragma once

#include "payoff/PayoffNode.h"
#include "common/types.h"

namespace payoff {

class Payoff {
   public:
    virtual ~Payoff() = default;
};

class CashPayment final : public Payoff {
   public:
    CashPayment(PayoffNodePtr amount, const Date settlementDate)
        : _amount(std::move(amount)), _settlementDate(settlementDate){};

    const PayoffNodePtr& getAmount() const { return _amount; };
    Date getSettlementDate() const { return _settlementDate; };

   private:
    const PayoffNodePtr _amount;
    const Date _settlementDate;
};
}  // namespace payoff