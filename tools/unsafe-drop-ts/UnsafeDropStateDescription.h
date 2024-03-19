#ifndef UNSAFE_DROP_STATE_DESCRIPTION_H
#define UNSAFE_DROP_STATE_DESCRIPTION_H

#include "phasar/PhasarLLVM/DataFlow/IfdsIde/Problems/IDETypeStateAnalysis.h"
#include "phasar/PhasarLLVM/DataFlow/IfdsIde/Problems/TypeStateDescriptions/TypeStateDescription.h"
#include "phasar.h"

#include <map>
#include <set>
#include <string>

#include "llvm/ADT/StringRef.h"

namespace psr
{

    /**
     * The states of the Finite State Machine,
     * forming a flat lattice.
     *
     *                          BOT = all information is possible
     *
     * UNINIT    GET_PTR    UNSAFE_CONSTRUCT    DROP    USE    ERROR
     *
     *                          TOP = no information is avilable
     *
     * Note that TOP and BOT are revesed in all of PhASAR.
     */
    // TODO: add more destinct "error" cases for UAF and DF as they
    // are not really errors in this sense but the patterns we want to detect
    enum class UnsafeDropState : uint8_t
    {
        BOT = 0,
        UNINIT = 1,
        GET_PTR = 2,
        UNSAFE_CONSTRUCT = 3,
        DROP = 4,
        USE = 5,
        ERROR = 6,
        TOP = UINT8_MAX,
    };

    llvm::StringRef to_string(UnsafeDropState State) noexcept;

    /**
     * Lattice implementation for UnsafeDropState,
     * forming a flat lattice.
     *
     */
    template <>
    struct JoinLatticeTraits<UnsafeDropState>
    {
        static constexpr UnsafeDropState top() noexcept
        {
            return UnsafeDropState::TOP;
        }
        static constexpr UnsafeDropState bottom() noexcept
        {
            return UnsafeDropState::BOT;
        }
        static constexpr UnsafeDropState join(UnsafeDropState L,
                                              UnsafeDropState R) noexcept
        {
            if (L == top() || R == bottom())
            {
                return R;
            }
            if (L == bottom() || R == top())
            {
                return L;
            }
            return bottom();
        }
    };

    class UnsafeDropStateDescription
        : public TypeStateDescription<UnsafeDropState>
    {
    private:
        HelperAnalyses &HA;

    public:
        UnsafeDropStateDescription(HelperAnalyses &HA) : HA(HA) {}

        using TypeStateDescription::getNextState;

        [[nodiscard]] bool isFactoryFunction(llvm::StringRef F) const override;

        [[nodiscard]] bool isConsumingFunction(llvm::StringRef F) const override;

        [[nodiscard]] bool isAPIFunction(llvm::StringRef F) const override;

        [[nodiscard]] TypeStateDescription::State
        getNextState(llvm::StringRef Tok,
                     TypeStateDescription::State S) const override;

        [[nodiscard]] std::string getTypeNameOfInterest() const override;

        [[nodiscard]] std::set<int>
        getConsumerParamIdx(llvm::StringRef F) const override;

        [[nodiscard]] std::set<int>
        getFactoryParamIdx(llvm::StringRef F) const override;

        [[nodiscard]] TypeStateDescription::State bottom() const override
        {
            return UnsafeDropState::BOT;
        }

        [[nodiscard]] TypeStateDescription::State top() const override
        {
            return UnsafeDropState::TOP;
        }

        [[nodiscard]] TypeStateDescription::State uninit() const override
        {
            return UnsafeDropState::UNINIT;
        }

        [[nodiscard]] TypeStateDescription::State start() const override
        {
            return UnsafeDropState::GET_PTR;
        }

        [[nodiscard]] TypeStateDescription::State error() const override
        {
            return UnsafeDropState::ERROR;
        }
    }; // class UnsafeDropStateDescription

    extern template class IDETypeStateAnalysis<UnsafeDropStateDescription>;

    /**
     * The transition tokens of the Finite State machine, i.e. the functions called.
     * The STAR token represents all API that are not relevant.
     */
    enum class UnsafeDropToken : uint8_t
    {
        STAR = 0,
        GET_PTR = 1,
        UNSAFE_CONSTRUCT = 2,
        DROP = 3,
        USE = 4,
    };

} // namespace psr

#endif // UNSAFE_DROP_STATE_DESCRIPTION_H