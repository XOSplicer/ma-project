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
            return "UnsafeDropToken::STAR";
        case UnsafeDropToken::GET_PTR:
            return "UnsafeDropToken::GET_PTR";
        case UnsafeDropToken::UNSAFE_CONSTRUCT:
            return "UnsafeDropToken::UNSAFE_CONSTRUCT";
        case UnsafeDropToken::DROP:
            return "UnsafeDropToken::DROP";
        case UnsafeDropToken::USE:
            return "UnsafeDropToken::USE";
        }
        llvm::report_fatal_error("received unknown token!");
    }

    llvm::StringRef to_string(UnsafeDropState State) noexcept
    {
        switch (State)
        {
        case UnsafeDropState::BOT:
            return "UnsafeDropState::BOT";
        case UnsafeDropState::UNINIT:
            return "UnsafeDropState::UNINIT";
        case UnsafeDropState::RAW_REFERENCED:
            return "UnsafeDropState::RAW_REFERENCED";
        case UnsafeDropState::RAW_WRAPPED:
            return "UnsafeDropState::RAW_WRAPPED";
        case UnsafeDropState::DROPPED:
            return "UnsafeDropState::DROPPED";
        case UnsafeDropState::USED:
            return "UnsafeDropState::USED";
        case UnsafeDropState::UAF_ERROR:
            return "UnsafeDropState::UAF_ERROR";
        case UnsafeDropState::DF_ERROR:
            return "UnsafeDropState::DF_ERROR";
        case UnsafeDropState::TS_ERROR:
            return "UnsafeDropState::TS_ERROR";
        case UnsafeDropState::TOP:
            return "UnsafeDropState::TOP";
        }

        llvm::report_fatal_error("received unknown state!");
    }

    UnsafeDropState Delta(UnsafeDropToken token, UnsafeDropState state, bool unsafe_construct_as_factory)
    {
        switch (token)
        {
        case UnsafeDropToken::STAR:
            // identity
            return state;
        case UnsafeDropToken::GET_PTR:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::RAW_REFERENCED;
            case UnsafeDropState::RAW_REFERENCED:
                return UnsafeDropState::RAW_REFERENCED;
            case UnsafeDropState::RAW_WRAPPED:
                return UnsafeDropState::RAW_WRAPPED;
            case UnsafeDropState::DROPPED:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::USED:
                return UnsafeDropState::USED;
            case UnsafeDropState::UAF_ERROR:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::DF_ERROR:
                return UnsafeDropState::DF_ERROR;
            case UnsafeDropState::TS_ERROR:
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::UNSAFE_CONSTRUCT:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
            case UnsafeDropState::UNINIT:

                /*

                    as a second analysis ot overcome the limitation that not all functions are called in the same object
                    but some are called on the original and some are called on the wrapped object
                    - adapt this transition to be RAW_WRAPPED
                    - allow UNSAFE_CONSTRUCT functions to be factory functions
                    then run both the original analysis and the second and check if statement
                        that calls a UNSAFE_CONSTRUCT function has a dataflow fact that is RAW_REFERENCED and uses it as an argument
                */

                if (unsafe_construct_as_factory)
                {
                    return UnsafeDropState::RAW_WRAPPED;
                }
                else
                {
                    return UnsafeDropState::TS_ERROR;
                }
            case UnsafeDropState::RAW_REFERENCED:
                return UnsafeDropState::RAW_WRAPPED;
            case UnsafeDropState::RAW_WRAPPED:
                return UnsafeDropState::RAW_WRAPPED;
            case UnsafeDropState::DROPPED:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::USED:
                return UnsafeDropState::USED;
            case UnsafeDropState::UAF_ERROR:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::DF_ERROR:
                return UnsafeDropState::DF_ERROR;
            case UnsafeDropState::TS_ERROR:
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::DROP:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
            case UnsafeDropState::UNINIT:
                //return UnsafeDropState::DROPPED;
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::RAW_REFERENCED:
                return UnsafeDropState::DROPPED;
            case UnsafeDropState::RAW_WRAPPED:
                return UnsafeDropState::DROPPED;
            case UnsafeDropState::DROPPED:
                return UnsafeDropState::DF_ERROR;
            case UnsafeDropState::USED:
                return UnsafeDropState::DROPPED;
            case UnsafeDropState::UAF_ERROR:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::DF_ERROR:
                return UnsafeDropState::DF_ERROR;
            case UnsafeDropState::TS_ERROR:
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        case UnsafeDropToken::USE:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
            case UnsafeDropState::UNINIT:
                //return UnsafeDropState::USED;
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::RAW_REFERENCED:
                return UnsafeDropState::USED;
            case UnsafeDropState::RAW_WRAPPED:
                return UnsafeDropState::USED;
            case UnsafeDropState::DROPPED:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::USED:
                return UnsafeDropState::USED;
            case UnsafeDropState::UAF_ERROR:
                return UnsafeDropState::UAF_ERROR;
            case UnsafeDropState::DF_ERROR:
                return UnsafeDropState::DF_ERROR;
            case UnsafeDropState::TS_ERROR:
                return UnsafeDropState::TS_ERROR;
            case UnsafeDropState::TOP:
                return UnsafeDropState::TOP;
            }
            llvm::report_fatal_error("received unknown state!");
            break;
        }
        llvm::report_fatal_error("received unknown token!");
    }

    typedef UnsafeDropStateDescription::FnInfo FnInfo;

    const llvm::StringMap<FnInfo> &getInterestingFns() noexcept
    {
        // TODO: add more interesting functions that match the substring checks of funcNameToToken
        // Return value is modeled as -1
        static const llvm::StringMap<FnInfo> InterestingFns = {
            {"<alloc::boxed::Box<u32>>::from_raw", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT}},
            {"<alloc::boxed::Box<u32>>::into_raw", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR }},
            {"<alloc::raw_vec::RawVec<i32> as core::ops::drop::Drop>::drop", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"<alloc::raw_vec::RawVec<u8> as core::ops::drop::Drop>::drop", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"<alloc::vec::Vec<f32>>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR}},
            {"<alloc::vec::Vec<f32>>::from_raw_parts", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0, 1}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT}},
            {"<alloc::vec::Vec<i32>>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR}},
            {"<alloc::vec::Vec<u8>>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR}},
            {"<alloc::vec::Vec<u8>>::from_raw_parts", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0, 1}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT}},
            {"<str>::as_mut_ptr", FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR}},
            {"core::mem::drop::<alloc::boxed::Box<u32>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::ptr::drop_in_place::<alloc::boxed::Box<u32>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::ptr::drop_in_place::<alloc::raw_vec::RawVec<f32>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::ptr::drop_in_place::<alloc::raw_vec::RawVec<u8>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::ptr::drop_in_place::<alloc::string::String>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::ptr::drop_in_place::<alloc::vec::Vec<u8>>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP}},
            {"core::slice::raw::from_raw_parts_mut::<u8>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT}},
            {"core::slice::raw::from_raw_parts::<f32>", FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT}},
        };
        return InterestingFns;
    }

    FnInfo UnsafeDropStateDescription::getFnInfo(llvm::StringRef F) const
    {
        // overwrite the is_facory_fn and factory_param_idxs if token == UnsafeDropToken::UNSAFE_CONSTRUCT}

        // first check if there is a concrete function that matches the function name
        auto fns = getInterestingFns();
        if (fns.count(F))
        {
            FnInfo Ret = fns.lookup(F);
            if (Ret.token == UnsafeDropToken::UNSAFE_CONSTRUCT && this->unsafe_construct_as_factory)
            {
                Ret.is_factory_fn = true;
                // NOTE: sret is handled in getFactoryParamIdx
                Ret.factory_param_idxs = {-1};
            }
            return Ret;
        }

        // next, try to guess a function signature
        if (F.contains("into_raw_parts") || F.contains("as_mut_ptr") || F.contains("into_raw"))
        {
            return FnInfo{.is_factory_fn = true, .factory_param_idxs = {-1}, .consumer_param_idxs = {}, .token = UnsafeDropToken::GET_PTR};
        }
        if (F.contains("from_raw_parts") || F.contains("from_raw"))
        {
            FnInfo Ret = FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0, 1}, .token = UnsafeDropToken::UNSAFE_CONSTRUCT};
            if (Ret.token == UnsafeDropToken::UNSAFE_CONSTRUCT && this->unsafe_construct_as_factory)
            {
                Ret.is_factory_fn = true;
                // NOTE: sret is handled in getFactoryParamIdx
                Ret.factory_param_idxs = {-1};
            }
            return Ret;
        }
        if (F.contains("drop_in_place") || F.contains("core::ops::drop::Drop") || F.contains("core::mem::drop"))
        {
            return FnInfo{.is_factory_fn = false, .factory_param_idxs = {}, .consumer_param_idxs = {0}, .token = UnsafeDropToken::DROP};
        }

        // TODO: F is already demangled here and can therefore not be found in the IRDB sometimes
        auto fn = this->HA.getProjectIRDB().getFunction(F);
        if (!fn)
        {
            PHASAR_LOG_LEVEL(DEBUG, "warning: getFnInfo: lookup of function failed, falling back to STAR Token, F=" << F);
            return FnInfo{
                .is_factory_fn = false,
                .factory_param_idxs = {},
                .consumer_param_idxs = {},
                .token = UnsafeDropToken::STAR,
            };
        }

        // fall back to a USE token
        auto consumer_param_idxs = std::set<int>();
        for (auto x = 0; x < fn->arg_size(); ++x)
        {
            consumer_param_idxs.emplace(x);
        }
        return FnInfo{
            .is_factory_fn = false,
            .factory_param_idxs = {},
            .consumer_param_idxs = consumer_param_idxs,
            .token = UnsafeDropToken::USE,
        };
    }

    UnsafeDropToken UnsafeDropStateDescription::funcNameToToken(llvm::StringRef F) const
    {
        return this->getFnInfo(F).token;
    }

    bool UnsafeDropStateDescription::isFactoryFunction(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "isFactoryFunction: " << F);

        auto fnInfo = this->getFnInfo(F);
        return fnInfo.is_factory_fn || fnInfo.token == UnsafeDropToken::GET_PTR;
    }

    bool UnsafeDropStateDescription::isConsumingFunction(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "isConsumingFunction: " << F);
        auto fnInfo = this->getFnInfo(F);
        return !fnInfo.is_factory_fn;
    }

    bool UnsafeDropStateDescription::isAPIFunction(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "isAPIFunction: " << F);
        return true;
    }

    UnsafeDropState
    UnsafeDropStateDescription::getNextState(llvm::StringRef Tok,
                                             UnsafeDropState S) const
    {
        auto token = this->funcNameToToken(Tok);
        auto next = Delta(token, S, this->unsafe_construct_as_factory);
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
        auto fnInfo = this->getFnInfo(F);
        return fnInfo.consumer_param_idxs;
    }

    std::set<int>
    UnsafeDropStateDescription::getFactoryParamIdx(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "getFactoryParamIdx: " << F);
        auto fnInfo = this->getFnInfo(F);
        if (fnInfo.is_factory_fn)
        {
            auto fn_opt = this->demangled_lookup.lookup(F);
            if (!fn_opt.has_value())
            {
                PHASAR_LOG_LEVEL(DEBUG, "Warning: getFactoryParamIdx: DemangledLookup::lookup failed for F=" << F);
            }
            // try to handle sret if we can get the information
            if (fn_opt.has_value())
            {
                auto fn = fn_opt.value();
                int arg_num = 0;
                for (auto &arg : fn->args())
                {
                    // check if sret is used
                    if (arg.hasStructRetAttr())
                    {
                        PHASAR_LOG_LEVEL(DEBUG, "getFactoryParamIdx: Setting sret arg as factory param F=" << F << " arg_num=" << arg_num);
                        fnInfo.factory_param_idxs.emplace(arg_num);
                    }
                    arg_num++;
                }
            }
        }
        return fnInfo.factory_param_idxs;
    }

} // namespace psr
