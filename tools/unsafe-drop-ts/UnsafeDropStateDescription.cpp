#include <string>
#include "UnsafeDropStateDescription.h"
#include "phasar/PhasarLLVM/DB/LLVMProjectIRDB.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/ErrorHandling.h"
#include "phasar.h"
#include "rustc_demangle.h"

#define RUSTC_DEMANGLE_STRING_OUT_LEN 1024

namespace psr
{

    // NOTE: demangle is usually not needed as phasar already uses the
    // function name annotated in the debug information of the ll file
    std::string rustc_demangle_string(llvm::StringRef sr)
    {
        const std::string s = sr.str();
        const char *mangled = s.c_str();
        char out[RUSTC_DEMANGLE_STRING_OUT_LEN] = {0};
        const int no_err = rustc_demangle(mangled, out, RUSTC_DEMANGLE_STRING_OUT_LEN);
        if (!no_err)
        {
            PHASAR_LOG_LEVEL(DEBUG, "Could not demangle string with rustc_demangle, falling back to identity str=" << sr);
            return sr.str();
        }
        auto out_string = std::string(out);
        PHASAR_LOG_LEVEL(DEBUG, "demangled '" << sr << "' to '" << out_string << "'");
        return out_string;
    }

    template class IDETypeStateAnalysis<UnsafeDropStateDescription>;

    llvm::StringRef to_string(UnsafeDropToken Token) noexcept
    {
        switch (Token)
        {
        case UnsafeDropToken::STAR:
            return "STAR";
            break;
        case UnsafeDropToken::GET_PTR:
            return "GET_PTR";
            break;
        case UnsafeDropToken::UNSAFE_CONSTRUCT:
            return "UNSAFE_CONSTRUCT";
            break;
        case UnsafeDropToken::DROP:
            return "DROP";
            break;
        case UnsafeDropToken::USE:
            return "USE";
            break;
        }
        llvm::report_fatal_error("received unknown token!");
    }

    llvm::StringRef to_string(UnsafeDropState State) noexcept
    {
        switch (State)
        {
        case UnsafeDropState::TOP:
            return "TOP";
            break;
        case UnsafeDropState::UNINIT:
            return "UNINIT";
            break;
        case UnsafeDropState::GET_PTR:
            return "GET_PTR";
            break;
        case UnsafeDropState::UNSAFE_CONSTRUCT:
            return "UNSAFE_CONSTRUCT";
            break;
        case UnsafeDropState::DROP:
            return "DROP";
            break;
        case UnsafeDropState::USE:
            return "USE";
            break;
        case UnsafeDropState::ERROR:
            return "ERROR";
            break;
        case UnsafeDropState::BOT:
            return "BOT";
            break;
        }

        llvm::report_fatal_error("received unknown state!");
    }

    UnsafeDropState Delta(UnsafeDropToken token, UnsafeDropState state)
    {
        // llvm::report_fatal_error("UnsafeDropStateDeltaFn unimplemented!");
        //  TODO: add correct transition destinations
        switch (token)
        {
        case UnsafeDropToken::STAR:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::UNINIT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::GET_PTR;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::UNSAFE_CONSTRUCT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::DROP;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::USE;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
                break;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::GET_PTR:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::GET_PTR;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
                break;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::UNSAFE_CONSTRUCT:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::UNSAFE_CONSTRUCT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::UNSAFE_CONSTRUCT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
                break;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::DROP:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::DROP;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::DROP;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
                break;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::USE:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::GET_PTR;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::USE;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::USE;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::USE;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::ERROR;
                break;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
                break;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        }
        llvm::report_fatal_error("received unknown token!");
    }

    UnsafeDropToken funcNameToToken(llvm::StringRef F)
    {
        //std::string s = rustc_demangle_string(F);
        //auto demangled = llvm::StringRef(s);
        auto demangled = F;
        // TODO: implement
        if (demangled.contains("into_raw_parts") || demangled.contains("as_mut_ptr"))
        {
            return UnsafeDropToken::GET_PTR;
        }
        if (demangled.contains("from_raw_parts") || demangled.contains("from_raw"))
        {
            return UnsafeDropToken::UNSAFE_CONSTRUCT;
        }
        if (demangled.contains("drop_in_place"))
        {
            return UnsafeDropToken::DROP;
        }
        return UnsafeDropToken::USE;
    }

    struct FnInfo
    {
        bool is_factory_fn;
        std::set<int> factory_param_idxs;
        std::set<int> consumer_param_idxs;
    };

    const llvm::StringMap<FnInfo> &getInterestingFns() noexcept
    {
        // Return value is modeled as -1
        static const llvm::StringMap<FnInfo> InterestingFns = {
            {"<str>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}}},
            {"<alloc::vec::Vec<u8>>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}}},
            // how to handle sret?
            {"<alloc::vec::Vec<u8>>::from_raw_parts", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0,1}}},
            {"core::ptr::drop_in_place::<alloc::string::String>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}}},
            {"core::ptr::drop_in_place::<alloc::vec::Vec<u8>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}}},
        };
        return InterestingFns;
    }

    bool UnsafeDropStateDescription::isFactoryFunction(llvm::StringRef F) const
    {
        // TODO: implement better heuristics and correct methods to model
        PHASAR_LOG_LEVEL(DEBUG, "isFactoryFunction: " << F);
        if (isAPIFunction(F)) {
            return getInterestingFns().lookup(F).is_factory_fn;
        }
        return funcNameToToken(F) == UnsafeDropToken::GET_PTR;
    }

    bool UnsafeDropStateDescription::isConsumingFunction(llvm::StringRef F) const
    {
        // TODO: implement
        PHASAR_LOG_LEVEL(DEBUG, "isConsumingFunction: " << F);
        if (isAPIFunction(F)) {
            return !getInterestingFns().lookup(F).is_factory_fn;
        }
        return true;
    }

    bool UnsafeDropStateDescription::isAPIFunction(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "isAPIFunction: " << F);
        // TODO: implement better heuristics and correct methods to model
        return getInterestingFns().count(F);
    }

    UnsafeDropState
    UnsafeDropStateDescription::getNextState(llvm::StringRef Tok,
                                             UnsafeDropState S) const
    {
        auto token = funcNameToToken(Tok);
        auto next = Delta(token, S);
        PHASAR_LOG_LEVEL(DEBUG, "getNextState: fun=" << Tok << " token=" << to_string(token) << " state=" << to_string(S) << " next=" << to_string(next));
        return next;
    }

    std::string UnsafeDropStateDescription::getTypeNameOfInterest() const
    {
        // any type is considered
        return "";
    }

    std::set<int>
    UnsafeDropStateDescription::getConsumerParamIdx(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "getConsumerParamIdx: " << F);
        if (isAPIFunction(F)) {
            return getInterestingFns().lookup(F).consumer_param_idxs;
        }
        return {};
    }

    std::set<int>
    UnsafeDropStateDescription::getFactoryParamIdx(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "getFactoryParamIdx: " << F);
        // TODO: check if sret is used
        if (isAPIFunction(F)) {
            return getInterestingFns().lookup(F).factory_param_idxs;
        }
        return {-1};
    }

} // namespace psr
