#include "UnsafeDropStateDescription.h"

#include "phasar/PhasarLLVM/DB/LLVMProjectIRDB.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/ErrorHandling.h"

#include <string>

namespace psr
{

    template class IDETypeStateAnalysis<UnsafeDropStateDescription>;

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

    UnsafeDropState UnsafeDropStateDeltaFn(UnsafeDropToken token, UnsafeDropState state)
    {
        llvm::report_fatal_error("UnsafeDropStateDeltaFn unimplemented!");
        // TODO: add correct transition destinations
        switch (token)
        {
        case UnsafeDropToken::STAR:
            switch (state)
            {
            case UnsafeDropState::BOT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNINIT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::BOT;
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
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::BOT;
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
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::BOT;
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
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::BOT;
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
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::GET_PTR:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::UNSAFE_CONSTRUCT:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::DROP:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::USE:
                return UnsafeDropState::BOT;
                break;
            case UnsafeDropState::ERROR:
                return UnsafeDropState::BOT;
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

    bool UnsafeDropStateDescription::isFactoryFunction(llvm::StringRef F) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return false;
    }

    bool UnsafeDropStateDescription::isConsumingFunction(llvm::StringRef F) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return false;
    }

    bool UnsafeDropStateDescription::isAPIFunction(llvm::StringRef F) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return false;
    }

    UnsafeDropState
    UnsafeDropStateDescription::getNextState(llvm::StringRef Tok,
                                             UnsafeDropState S) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return UnsafeDropState::BOT;
    }

    std::string UnsafeDropStateDescription::getTypeNameOfInterest() const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return "";
    }

    std::set<int>
    UnsafeDropStateDescription::getConsumerParamIdx(llvm::StringRef F) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return {};
    }

    std::set<int>
    UnsafeDropStateDescription::getFactoryParamIdx(llvm::StringRef F) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented!");
        return {};
    }

} // namespace psr
