#include "ConsoleApp.h"

#include <sstream>
#include <iterator>
#include <iostream>

namespace qcli
{
   std::string ConsoleApp::OwnName;

   ConsoleApp::ConsoleApp()
   {
   }

   ConsoleApp::~ConsoleApp()
   {
      for (auto& command : mCommands)
         delete command.second;
   }

   ConsoleApp::CliCommand& ConsoleApp::addCommand(CommandCb exec, const std::string& syntax, const std::string& section)
   {
      std::istringstream iss(syntax);
      std::list<std::string> tokens;
      std::copy(std::istream_iterator<std::string, char>(iss), std::istream_iterator<std::string, char>(), std::back_inserter(tokens));

      std::string commandName;
      CliCommand *cliCommand = new CliCommand(exec, syntax);

      while (!tokens.empty())
      {
         auto token = tokens.front();
         tokens.pop_front();

         if (token[0] == L'<')
         {
            cliCommand->addParameter(token);
         }
         else if (token[0] == L'-')
         {
            if (!tokens.empty() && tokens.front()[0] == L'<')
            {
               auto token2 = tokens.front();
               tokens.pop_front();

               cliCommand->addOption(token, token2);
            }
            else
            {
               cliCommand->addOption(token);
            }
         }
         else
         {
            if (!commandName.empty())
               commandName += " ";

            commandName += token;
         }
      }

      cliCommand->setName(commandName);
      mCommands[commandName] = cliCommand;
      mOrderedCommands[section].push_back(cliCommand);

      return *cliCommand;
   }

   int ConsoleApp::executeOnce(int argc, char** argv)
   {
      OwnName = argv[0];
      std::string commandName;
      CliCommand *pCliCommand = nullptr;

      std::list<std::string> argList;

      for (int i = 1; i < argc; i++)
         argList.push_back(argv[i]);

      while (!argList.empty())
         if (iswalpha(argList.front()[0]))
         {
            if (!commandName.empty())
               commandName += " ";

            commandName += argList.front();

            auto cmdIt = mCommands.find(commandName);
            if (cmdIt != mCommands.end())
            {
               pCliCommand = cmdIt->second;
               argList.pop_front();
            }
            else
               break;
         }
         else
            break;

      if (commandName.empty() || commandName == "help")
      {
         printHelp();
         return 0;
      }

      if (!pCliCommand)
      {
         printf("Invalid command '%s'. For more details use '%s help'.\n", commandName.c_str(), argv[0]);
         return 1;
      }

      try
      {
         pCliCommand->execute(argList);
      }
      catch (std::exception& e)
      {
         printf("Command '%s' failed: %s\n", commandName.c_str(), e.what());
         return 1;
      }

      return 0;
   }

   int ConsoleApp::execute(int, char** argv)
   {
      OwnName = argv[0];
      printHelp();

      while ( true )
      {
         std::string line;
         std::string commandName;
         CliCommand *pCliCommand = nullptr;

         std::cout << "\n>";
         std::getline( std::cin, line );

         std::list<std::string> argList;
         std::istringstream buf( line );
         argList.assign(std::istream_iterator<std::string>( buf ), std::istream_iterator<std::string>());

         while (!argList.empty())
         {
            if (iswalpha(argList.front()[0]))
            {
               // Check if there is exactly one command that starts with commandName
               int count = 0;
               CliCommand *tmpCmd = nullptr;
               std::string tmpCmdName = commandName;

               if (!tmpCmdName.empty())
               {
                  tmpCmdName += " ";
               }

               tmpCmdName += argList.front();

               for (auto& c : mCommands)
               {
                  if (c.first == tmpCmdName)
                  {
                     tmpCmd = c.second;
                     count = 1;
                     break;
                  }

                  if (c.first.size() > tmpCmdName.size() and c.first.substr(0, tmpCmdName.size()) == tmpCmdName)
                  {
                     tmpCmd = c.second;
                     ++count;
                  }
               }

               if (tmpCmd)
               {
                  if (count == 1)
                  {
                     pCliCommand = tmpCmd;
                  }

                  argList.pop_front();
                  commandName = tmpCmdName;
               }
               else
               {
                  if (commandName.empty())
                  {
                     commandName = tmpCmdName;
                  }

                  break;
               }
            }
            else
            {
               break;
            }
         }

         if (commandName.empty())
         {
            continue;
         }
         else if (commandName == "help")
         {
            printHelp();
         }
         else if (!pCliCommand)
         {
            printf("Invalid command '%s'. For more details use '%s help'.\n", commandName.c_str(), argv[0]);
         }
         else
         {
            try
            {
               pCliCommand->execute(argList);
            }
            catch (std::exception& e)
            {
               printf("Command '%s' failed: %s\n", commandName.c_str(), e.what());
            }
         }
      }

      return 0;
   }

   void ConsoleApp::printHelp()
   {
      printf("Supported commands:\n");

      size_t cmdMaxSize = 0;
      for (auto& command : mCommands)
         if (cmdMaxSize < command.second->getSyntax().size())
            cmdMaxSize = command.second->getSyntax().size();

      for (auto& commandSection : mOrderedCommands)
      {
         printf("\n%s\n", commandSection.first.c_str());

         for (auto& command : commandSection.second)
         {
            if (command->getSummary().empty())
               printf("   %s\n", command->getSyntax().c_str());
            else
               printf("   %-*s  - %s\n", (int)cmdMaxSize, command->getSyntax().c_str(), command->getSummary().c_str());
         }
      }
   }

   ConsoleApp::CliCommand::CliCommand(CommandCb exec, const std::string& syntax) :
      mExec(exec),
      mSyntax(syntax)
   {
   }

   void ConsoleApp::CliCommand::execute(std::list<std::string>& argList)
   {
      std::vector<std::string> consoleParameters;
      CliArgs commandArgs;

      // Process options
      while (!argList.empty())
      {
         auto arg = argList.front();
         argList.pop_front();

         if (arg[0] == L'-')
         {
            switch (processGlobalOption(arg))
            {
            case Continue: continue;
            case Terminate: return;
            default:;
            }

            auto argDescIt = mOptions.find(arg);
            if (argDescIt == mOptions.end())
            {
               printf("Invalid option '%s' for command '%s'. For more details use '%s help' or '%s %s -h'.\n", arg.c_str(), mCommandName.c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());
               return;
            }

            if(argDescIt->second.hasValue)
            {
               if (argList.empty())
               {
                  printf("Missing value '%s' for option '%s'. For more details use '%s help' or '%s %s -h'.\n", argDescIt->second.valueName.c_str(), argDescIt->second.name.c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());
                  return;
               }

               commandArgs.addOption(argDescIt->second.name);
               commandArgs.addParameter(argDescIt->second.valueName, argList.front());
               argList.pop_front();
            }
            else
               commandArgs.addOption(argDescIt->second.name);
         }
         else
            consoleParameters.push_back(arg);
      }

      // Process parameters
      for (size_t i = 0; i < mParameters.size(); i++)
      {
         if (consoleParameters.size() <= i)
         {
            printf("Missing parameter '%s'. For more details use '%s help' or '%s %s -h'.\n", mParameters[i].c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());
            return;
         }

         commandArgs.addParameter(mParameters[i], consoleParameters[i]);
      }

      // Execute command
      mExec(commandArgs);
   }

   ConsoleApp::CliCommand::Action ConsoleApp::CliCommand::processGlobalOption(std::string& option)
   {
      if(option == "-h")
      {
         if (!getSummary().empty())
         {
            printf("%s\n\n", getSummary().c_str());
         }

         printf("NAME\n   %s\n\n", getName().c_str());
         printf("SYNTAX\n   %s\n\n", getSyntax().c_str());


         if (!getDescription().empty())
         {
            printf("%s\n\n", getDescription().c_str());
         }

         return Terminate;
      }

      return Ignore;
   }

} // namespace qcli
