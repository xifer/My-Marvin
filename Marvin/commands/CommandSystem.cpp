#include "CommandSystem.h"

#include "../Bot.h"
#include "../Common.h"
#include "../Debug.h"
#include "BaseDuelCommands.h"
#include "CombatRoleCommands.h"
#include "CommandsCommand.h"
#include "DelimiterCommand.h"
#include "HelpCommand.h"
#include "LockCommand.h"
#include "LockFreqCommand.h"
#include "ModListCommand.h"
#include "StatusCommands.h"
#include "ConsumableCommands.h"
#include "SetFreqCommand.h"
#include "SetShipCommand.h"
#include "SetArenaCommand.h"
#include "SwarmCommand.h"
#include "HSCommands.h"
#include "EGCommands.h"
#include "UFOCommand.h"

namespace marvin {

constexpr int kArenaSecurityLevel = 5;
const std::unordered_map<std::string, int> kOperators = {
    {"tm_master", 10}, {"baked cake", 10}, {"x-demo", 10}, {"lyra.", 5}, {"profile", 5}, {"monkey", 5},
    {"neostar", 5},    {"geekgrrl", 5},    {"sed", 5},     {"sk", 5},    {"b.o.x.", 10}};

CommandSystem::CommandSystem(Zone zone) {

  RegisterCommand(std::make_shared<HelpCommand>());
  RegisterCommand(std::make_shared<CommandsCommand>());
  RegisterCommand(std::make_shared<DelimiterCommand>());
  RegisterCommand(std::make_shared<ModListCommand>());

  RegisterCommand(std::make_shared<LockCommand>());
  RegisterCommand(std::make_shared<UnlockCommand>());

  RegisterCommand(std::make_shared<SetShipCommand>());
  RegisterCommand(std::make_shared<SetFreqCommand>());
  RegisterCommand(std::make_shared<SetArenaCommand>());

  RegisterCommand(std::make_shared<LockFreqCommand>());
  RegisterCommand(std::make_shared<UnlockFreqCommand>());

  RegisterCommand(std::make_shared<MultiCommand>());
  RegisterCommand(std::make_shared<MultiOffCommand>());
  RegisterCommand(std::make_shared<CloakCommand>());
  RegisterCommand(std::make_shared<CloakOffCommand>());
  RegisterCommand(std::make_shared<StealthCommand>());
  RegisterCommand(std::make_shared<StealthOffCommand>());
  RegisterCommand(std::make_shared<XRadarCommand>());
  RegisterCommand(std::make_shared<XRadarOffCommand>());
  RegisterCommand(std::make_shared<AntiWarpCommand>());
  RegisterCommand(std::make_shared<AntiWarpOffCommand>());


  RegisterCommand(std::make_shared<RepelCommand>());
  RegisterCommand(std::make_shared<RepelOffCommand>());
  RegisterCommand(std::make_shared<BurstCommand>());
  RegisterCommand(std::make_shared<BurstOffCommand>());
  RegisterCommand(std::make_shared<DecoyCommand>());
  RegisterCommand(std::make_shared<DecoyOffCommand>());
  RegisterCommand(std::make_shared<RocketCommand>());
  RegisterCommand(std::make_shared<RocketOffCommand>());
  RegisterCommand(std::make_shared<BrickCommand>());
  RegisterCommand(std::make_shared<BrickOffCommand>());
  RegisterCommand(std::make_shared<PortalCommand>());
  RegisterCommand(std::make_shared<PortalOffCommand>());
  RegisterCommand(std::make_shared<ThorCommand>());
  RegisterCommand(std::make_shared<ThorOffCommand>());
  RegisterCommand(std::make_shared<UFOCommand>());
  RegisterCommand(std::make_shared<UFOOffCommand>());

   switch (zone) {
    case Zone::Devastation: {
      RegisterCommand(std::make_shared<BDPublicCommand>());
      RegisterCommand(std::make_shared<BDPrivateCommand>());
      RegisterCommand(std::make_shared<StartBDCommand>());
      RegisterCommand(std::make_shared<StopBDCommand>());
      RegisterCommand(std::make_shared<HoldBDCommand>());
      RegisterCommand(std::make_shared<ResumeBDCommand>());
      RegisterCommand(std::make_shared<AnchorCommand>());
      RegisterCommand(std::make_shared<RushCommand>());
      RegisterCommand(std::make_shared<SwarmCommand>());
      RegisterCommand(std::make_shared<SwarmOffCommand>());
      break;
    }
    case Zone::Hyperspace: {
      RegisterCommand(std::make_shared<HSFlagCommand>());
      RegisterCommand(std::make_shared<HSFlagOffCommand>());
      RegisterCommand(std::make_shared<HSBuyCommand>());
      RegisterCommand(std::make_shared<HSSellCommand>());
      RegisterCommand(std::make_shared<HSShipStatusCommand>());
      RegisterCommand(std::make_shared<AnchorCommand>());
      RegisterCommand(std::make_shared<RushCommand>());
      break;
    }
    case Zone::ExtremeGames: {
      RegisterCommand(std::make_shared<EGPCommand>());
      RegisterCommand(std::make_shared<EGLCommand>());
      RegisterCommand(std::make_shared<EGRCommand>());
      break;
    }
    default: {
      break;
    }
  }
}

int CommandSystem::GetSecurityLevel(const std::string& player) {
  auto iter = kOperators.find(player);

  if (iter != kOperators.end()) {
    return iter->second;
  }

  return 0;
}

bool CommandSystem::ProcessMessage(Bot& bot, ChatMessage& chat) {
  bool result = false;
  std::string& msg = chat.message;

  if (msg.empty()) return false;
  if (msg[0] != '!' && msg[0] != '.') return false;

  // ignore messages sent from self unless the message is pm'd to self
  // this works because pm'ing self causes a double message
  // so the function here can throw out the first message, and process the next one
  // if it is recieved within the 250ms limit 
  if (chat.player == bot.GetGame().GetName() && chat.type == ChatType::Private) {
   // reset the flag 
   if (time_.GetTime() > self_message_cooldown_) {
      ignore_self_message_ = true;
   }

   if (ignore_self_message_) {
      ignore_self_message_ = false;
      self_message_cooldown_ = time_.GetTime() + 250;
      return false;
   } 
  } 

  msg.erase(0, 1);

  std::vector<std::string> tokens = Tokenize(msg, ';');

  for (std::string current_msg : tokens) {
    std::size_t split = current_msg.find(' ');
    std::string trigger = Lowercase(current_msg.substr(0, split));
    std::string arg;

    if (split != std::string::npos) {
      arg = current_msg.substr(split + 1);
    }

    auto iter = commands_.find(trigger);
    if (iter != commands_.end()) {
      CommandExecutor& command = *iter->second;
      u32 access_request = (1 << (u32)chat.type);

      /* 
      * bitfield evaluation example (showing 4 bits):
      * chat type is 0000 for arena message
      * request_access evaluates to 0001
      * command access can be 0001 for arena or 0101 for both arena and public
      * 0001 & 0101 = 0001 which evaluates to true or non zero
      */
      if (access_request & command.GetAccess()) {
          int security_level = 0;

          if (chat.type == ChatType::Arena) {
            security_level = kArenaSecurityLevel;
          } else {
            auto op_iter = kOperators.find(Lowercase(chat.player));

            if (op_iter != kOperators.end()) {
              security_level = op_iter->second;
            }
          }

          if (security_level >= command.GetSecurityLevel()) {
            Blackboard& bb = bot.GetBlackboard();

            // If the command is lockable, bot is locked, and requester isn't an operator then ignore it.
            // if (!(command.GetFlags() & CommandFlag_Lockable) || !bb.ValueOr<bool>("CmdLock", false) ||
            if (!(command.GetFlags() & CommandFlag_Lockable) || !bb.GetCommandLock() || security_level > 0) {
              command.Execute(*this, bot, chat.player, arg);
              result = true;
            }
          }
      }
    }
  }

  return result;
}

const Operators& CommandSystem::GetOperators() const {
  return kOperators;
}

// Very simple tokenizer. Doesn't treat quoted strings as one token.
std::vector<std::string> Tokenize(std::string message, char delim) {
  std::vector<std::string> tokens;

  std::size_t pos = 0;

  while ((pos = message.find(delim)) != std::string::npos) {
    // Skip over multiple delims in a row
    if (pos > 0) {
      tokens.push_back(message.substr(0, pos));
    }

    message = message.substr(pos + 1);
  }

  if (!message.empty()) {
    tokens.push_back(message);
  }

  return tokens;
}

}  // namespace marvin
