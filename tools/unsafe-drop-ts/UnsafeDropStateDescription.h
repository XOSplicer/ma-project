#ifndef UNSAFE_DROP_STATE_DESCRIPTION_H
#define UNSAFE_DROP_STATE_DESCRIPTION_H

#include <map>
#include <set>
#include <string>
#include "llvm/ADT/StringRef.h"
#include "phasar.h"
#include "phasar/PhasarLLVM/DataFlow/IfdsIde/Problems/IDETypeStateAnalysis.h"
#include "phasar/PhasarLLVM/DataFlow/IfdsIde/Problems/TypeStateDescriptions/TypeStateDescription.h"

namespace psr
{

    /**
     * Reverse lookup map of mangled names to function definitions.
     * Computed for all functions in the LLVM IR.
     */
    class DemangledLookup
    {
    private:
        HelperAnalyses &HA;
        llvm::StringMap<const llvm::Function *> Map;

    public:
        /**
         * Constructor to build the reverse lookup table from the HelperAnalysis
         */
        DemangledLookup(HelperAnalyses &HA)
            : HA(HA),
              Map(llvm::StringMap<const llvm::Function *>())
        {
            for (const auto F : HA.getProjectIRDB().getAllFunctions())
            {
                const auto demangled = llvm::demangle(F->getName().str());
                this->Map.insert(std::make_pair(llvm::StringRef(demangled), F));
            }
        }

        /**
         * Try to lookup a demangled function name.
         * If found returns the function definition.
         */
        std::optional<const llvm::Function *> lookup(llvm::StringRef Demangled) const
        {
            if (this->Map.count(Demangled))
            {
                return this->Map.lookup(Demangled);
            }
            return std::nullopt;
        }
    }; // class DemangledLookup

    // template parameter to specialize which lattice implementation is used
    enum class UnsafeDropStateLatticeKind
    {
        FLAT,
        VERTICAL
    };

    /**
     * The states of the Finite State Machine,
     * forming a flat lattice.
     *
     *                          BOT = all information is possible
     *
     * UNINIT    RAW_REFERENCED    RAW_WRAPPED    DROP    USE   UAF_ERROR   DF_ERROR   ERROR
     *
     *                          TOP = no information is avilable
     *
     * Note that TOP and BOT are revesed in all of PhASAR.
     *
     * UAF and DF errors are not really errors in the senseof TS analysis but the patterns we want to detect
     */
    template <UnsafeDropStateLatticeKind LK>
    class UnsafeDropState_Wrapper
    {
    public:
        // enums cant be used as a template, so it is an inner class in a wrapper here
        enum class UnsafeDropState : int8_t
        {
            TOP = INT8_MIN,
            TS_ERROR = 0,
            UNINIT = 1,
            RAW_REFERENCED = 2,
            RAW_WRAPPED = 3,
            USED = 4,
            DROPPED = 5,
            UAF_ERROR = 6,
            DF_ERROR = 7,
            BOT = INT8_MAX,
        };
    };

    // switch lattice bahavior by changing the specialization of the wrapper type
    // using UnsafeDropState = UnsafeDropState_Wrapper<UnsafeDropStateLatticeKind::FLAT>::UnsafeDropState;
    using UnsafeDropState = UnsafeDropState_Wrapper<UnsafeDropStateLatticeKind::VERTICAL>::UnsafeDropState;

    llvm::StringRef to_string(UnsafeDropState State) noexcept;

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

    llvm::StringRef to_string(UnsafeDropToken Token) noexcept;

    /**
     * Lattice implementation for UnsafeDropState,
     * forming a flat lattice.
     *
     */
    template <>
    struct JoinLatticeTraits<UnsafeDropState_Wrapper<UnsafeDropStateLatticeKind::FLAT>::UnsafeDropState>
    {
        static constexpr UnsafeDropState top() noexcept { return UnsafeDropState::TOP; }
        static constexpr UnsafeDropState bottom() noexcept { return UnsafeDropState::BOT; }
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

    /**
     * Lattice implementation for UnsafeDropState,
     * forming a vertical lattice.
     *
     */
    template <>
    struct JoinLatticeTraits<UnsafeDropState_Wrapper<UnsafeDropStateLatticeKind::VERTICAL>::UnsafeDropState>
    {
        static constexpr UnsafeDropState top() noexcept { return UnsafeDropState::TOP; }
        static constexpr UnsafeDropState bottom() noexcept { return UnsafeDropState::BOT; }
        static constexpr UnsafeDropState join(UnsafeDropState L,
                                              UnsafeDropState R) noexcept
        {
            return std::max(L, R);
        }
    };

    class UnsafeDropStateDescription
        : public TypeStateDescription<UnsafeDropState>
    {
    public:
        struct FnInfo
        {
            bool is_factory_fn;
            std::set<int> factory_param_idxs;
            std::set<int> consumer_param_idxs;
            UnsafeDropToken token;
        };

    private:
        HelperAnalyses &HA;
        bool unsafe_construct_as_factory;
        DemangledLookup demangled_lookup;
        UnsafeDropStateDescription::FnInfo getFnInfo(llvm::StringRef F) const;
    public:
        UnsafeDropToken funcNameToToken(llvm::StringRef F) const;

    public:
        UnsafeDropStateDescription(HelperAnalyses &HA)
            : UnsafeDropStateDescription(HA, false) {}

        UnsafeDropStateDescription(HelperAnalyses &HA, bool unsafe_construct_as_factory)
            : HA(HA),
              unsafe_construct_as_factory(unsafe_construct_as_factory),
              demangled_lookup(DemangledLookup(HA))
        {
        }

        using TypeStateDescription::getNextState;

        // NOTE: F is an already demangled function name
        [[nodiscard]] bool isFactoryFunction(llvm::StringRef F) const override;

        // NOTE: F is an already demangled function name
        [[nodiscard]] bool isConsumingFunction(llvm::StringRef F) const override;

        // NOTE: F is an already demangled function name
        [[nodiscard]] bool isAPIFunction(llvm::StringRef F) const override;

        // NOTE: Tok is an already demangled function name
        [[nodiscard]] TypeStateDescription::State
        getNextState(llvm::StringRef Tok,
                     TypeStateDescription::State S) const override;

        [[nodiscard]] std::string getTypeNameOfInterest() const override;

        // NOTE: F is an already demangled function name
        [[nodiscard]] std::set<int>
        getConsumerParamIdx(llvm::StringRef F) const override;

        // NOTE: F is an already demangled function name
        [[nodiscard]] std::set<int>
        getFactoryParamIdx(llvm::StringRef F) const override;

        [[nodiscard]] TypeStateDescription::State bottom() const override { return UnsafeDropState::BOT; }
        [[nodiscard]] TypeStateDescription::State top() const override { return UnsafeDropState::TOP; }
        [[nodiscard]] TypeStateDescription::State uninit() const override { return UnsafeDropState::UNINIT; }
        [[nodiscard]] TypeStateDescription::State start() const override { return UnsafeDropState::RAW_REFERENCED; }
        [[nodiscard]] TypeStateDescription::State error() const override { return UnsafeDropState::TS_ERROR; }

        [[nodiscard]] virtual DataFlowAnalysisType analysisType() const override
        {
            return DataFlowAnalysisType::None;
        }
    }; // class UnsafeDropStateDescription

    extern template class IDETypeStateAnalysis<UnsafeDropStateDescription>;

} // namespace psr

#endif // UNSAFE_DROP_STATE_DESCRIPTION_H