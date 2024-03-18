#include "UnsafeDropStateDescription.h"

#include "phasar/PhasarLLVM/DB/LLVMProjectIRDB.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/Support/ErrorHandling.h"

#include "phasar.h"

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
                return UnsafeDropState::ERROR;
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

    bool UnsafeDropStateDescription::isFactoryFunction(llvm::StringRef F) const
    {
        // TODO: implement
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::isFactoryFunction");
        // return false;
        // TODO: implement better heuristics and correct methods to model
        PHASAR_LOG_LEVEL(DEBUG, "isFactoryFunction: " << F);
        return F.contains("new") || F.contains("from") || F.contains("with_capacity");
    }

    bool UnsafeDropStateDescription::isConsumingFunction(llvm::StringRef F) const
    {
        // TODO: implement
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::isConsumingFunction");
        // return false;
        PHASAR_LOG_LEVEL(DEBUG, "isConsumingFunction: " << F);
        return true;
    }

    bool UnsafeDropStateDescription::isAPIFunction(llvm::StringRef F) const
    {
        PHASAR_LOG_LEVEL(DEBUG, "isAPIFunction: " << F);
        // TODO: implement better heuristics and correct methods to model
        return F.contains("drop") || F.contains("from_raw_parts") || F.contains("into_raw_parts");
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::isAPIFunction");
    }

    UnsafeDropState
    UnsafeDropStateDescription::getNextState(llvm::StringRef Tok,
                                             UnsafeDropState S) const
    {
        // TODO: implement
        llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::getNextState");
        return UnsafeDropState::BOT;
    }

    std::string UnsafeDropStateDescription::getTypeNameOfInterest() const
    {
        // TODO: implement
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::getTypeNameOfInterest");
        return "";
    }

    std::set<int>
    UnsafeDropStateDescription::getConsumerParamIdx(llvm::StringRef F) const
    {
        // TODO: implement
        PHASAR_LOG_LEVEL(DEBUG, "getConsumerParamIdx: " << F);
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::getConsumerParamIdx");
        return {0};
    }

    std::set<int>
    UnsafeDropStateDescription::getFactoryParamIdx(llvm::StringRef F) const
    {
        // TODO: implement
        PHASAR_LOG_LEVEL(DEBUG, "getFactoryParamIdx: " << F);
        // llvm::report_fatal_error("unimplemented: UnsafeDropStateDescription::getFactoryParamIdx");
        return {-1};
    }

} // namespace psr
