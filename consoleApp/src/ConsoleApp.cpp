#include "ConsoleApp.h"

#include <sstream>
#include <iterator>
#include <iostream>
#include <limits>

namespace qcli
{

   static inline int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
   static inline int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }

   std::string ConsoleApp::OwnName;

   ConsoleApp::ConsoleApp(std::ostream& out) : mOut(out)
   {
       addCommand([=](std::ostream&, const CliArgs&) { this->printHelp(); }, "Help", "Help")
           .setSummary("Prints help message");
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

      if (commandName.empty())
      {
         printHelp();
         return 0;
      }

      if (!pCliCommand)
      {
         //printf("Invalid command '%s'. For more details use '%s help'.\n", commandName.c_str(), argv[0]);
         mOut << "Invalid command '" << commandName << "'. For more details use '" << argv[0] << " help'.\n";
         return 1;
      }

      try
      {
         pCliCommand->execute(argList, mOut);
      }
      catch (std::exception& e)
      {
         //printf("Command '%s' failed: %s\n", commandName.c_str(), e.what());
         mOut << "Command '" << commandName << "' failed: " << e.what() << "\n";
         return 1;
      }

      return 0;
   }


   size_t getMatchingChars(const std::string s1, const std::string s2)
   {
       size_t i;
       for (i = 0; i < s1.size() && i < s2.size(); i++)
           if (s1[i] != s2[i])
               return i;
       return i;
   }



   int ConsoleApp::executeCommand(const char* cmd)
   {
	   std::string line = cmd;
	   std::string commandName;
	   CliCommand* pCliCommand = nullptr;

	   std::list<std::string> argList;
	   std::istringstream buf(line);
	   argList.assign(std::istream_iterator<std::string>(buf), std::istream_iterator<std::string>());

	   while (!argList.empty())
	   {
		   if (iswalpha(argList.front()[0]))
		   {
			   // Check if there is exactly one command that starts with commandName
			   int count = 0;
			   CliCommand* tmpCmd = nullptr;
			   std::string tmpCmdName = commandName;

			   if (!tmpCmdName.empty())
			   {
				   tmpCmdName += " ";
			   }

			   tmpCmdName += argList.front();

			   for (auto& c : mCommands)
			   {
				   if (Stricmp(c.first.c_str(), tmpCmdName.c_str()) == 0)
				   {
					   tmpCmd = c.second;
					   count = 1;
					   break;
				   }

				  if (c.first.size() > tmpCmdName.size() and Stricmp(c.first.substr(0, tmpCmdName.size()).c_str(), tmpCmdName.c_str()) == 0)
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
		   return 1;
	   }
	   else if (!pCliCommand)
	   {
		   //printf("Invalid command '%s'. For more details call help.\n", commandName.c_str());
           mOut << "Invalid command '" << commandName << "'. For more details call help.\n";
	   }
	   else
	   {
		   try
		   {
			   pCliCommand->execute(argList, mOut);
		   }
		   catch (std::exception& e)
		   {
			   //printf("Command '%s' failed: %s\n", commandName.c_str(), e.what());
               mOut << "Command '" << commandName << "' failed: " << e.what() << "\n";
		   }
	   }

	   return 0;
   }

   int ConsoleApp::runCli(int, char** argv)
   {
	   OwnName = argv[0];
	   printHelp();

	   while (true)
	   {
		   std::string line;

		   std::cout << "\n>";
		   std::getline(std::cin, line);

           if (line[line.size() - 1] == '\t')
           {
               std::list<std::string> candidates;
               auto cmdSearch = line.substr(0, line.size() - 1);
               for (const auto& c : mCommands)
               {
                   c.second->getAutocompleteCandidates(cmdSearch, candidates);
               }

               for (auto i = candidates.begin(); i != candidates.end(); ++i)
                   std::cout << *i << std::endl;

               std::cout << "Autocomplete: " << getAutocomplete(candidates) << "|" << std::endl;
           }
           else
           {
               executeCommand(line.c_str());
           }

	   }
	   return 0;
   }

   void ConsoleApp::printHelp()
   {
      //printf("Supported commands:\n");
      mOut << "Supported commands:\n";

      size_t cmdMaxSize = 0;
      for (auto& command : mCommands)
         if (cmdMaxSize < command.second->getSyntax().size())
            cmdMaxSize = command.second->getSyntax().size();

      for (auto& commandSection : mOrderedCommands)
      {
         //printf("\n%s\n", commandSection.first.c_str());
         mOut << "\n" << commandSection.first << "\n";

         for (auto& command : commandSection.second)
         {
            if (command->getSummary().empty())
               //printf("   %s\n", command->getSyntax().c_str());
                mOut << "   " << command->getSyntax() << "\n";
            else
            {
                char buff[1024];
                sprintf(buff, "   %-*s  - %s\n", (int)cmdMaxSize, command->getSyntax().c_str(), command->getSummary().c_str());
                mOut << buff;
            }
         }
      }
   }

   ConsoleApp::CliCommand::CliCommand(CommandCb exec, const std::string& syntax) :
      mExec(exec),
      mSyntax(syntax)
   {
   }

   // trim from start
   static inline std::string& ltrim(std::string& s) {
       s.erase(s.begin(), std::find_if(s.begin(), s.end(),
           std::not1(std::ptr_fun<int, int>(std::isspace))));
       return s;
   }

   // trim from end
   static inline std::string& rtrim(std::string& s) {
       s.erase(std::find_if(s.rbegin(), s.rend(),
           std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
       return s;
   }

   // trim from both ends
   static inline std::string& trim(std::string& s) {
       return ltrim(rtrim(s));
   }


   bool ConsoleApp::CliCommand::canAutocomplete(std::string& cmd) const
   {
       return Strnicmp(mCommandName.c_str(), cmd.c_str(), std::min(mCommandName.size(), cmd.size())) == 0;
   }

   bool ConsoleApp::CliCommand::getAutocompleteCandidates(std::string& command, std::list<std::string>& candidates) const
   {
       auto cmd = trim(command);

       if (not canAutocomplete(cmd))
           return false;


       if (mCommandName.size() > cmd.size())
       {
           // Not enough words to autocomplete parameters
           candidates.push_back(mCommandName);
           return true;
       }

       return getParamsAutocomplete(cmd, candidates);
   }


   bool ConsoleApp::CliCommand::getParamsAutocomplete(std::string& cmd, std::list<std::string>& candidates) const
   {
       if (mParameters.empty())
       {
           return false;
       }

       auto prefix = getName();
       cmd = cmd.substr(prefix.size(), cmd.size());

       std::istringstream argsss(cmd);
       std::list<std::string> args;
       args.assign(std::istream_iterator<std::string>(argsss), std::istream_iterator<std::string>());

       if (args.size() > mParameters.size())
       {
           return false;
       }


       std::vector<Parameter>::const_iterator  param = mParameters.begin();
       std::string autocompleteArg;
       if (not args.empty())
       {
            param = mParameters.begin() + (args.size() - 1);
            autocompleteArg = args.back();
            args.pop_back();
       }

       if (param->autocompleteCb)
       {
           
           auto paramCandidates = param->autocompleteCb(autocompleteArg);
           if (paramCandidates.empty())
           {
               return false;
           }
 
           if (paramCandidates.size() == 1)
           {
               if (autocompleteArg == paramCandidates.front())
               {
                   if ((param + 1) != mParameters.end())
                   {
                       // The current parameter says that its slice is complete, place it back and ask the next one
                       args.push_back(autocompleteArg);
                       param++;
                       paramCandidates = param->autocompleteCb("");
                   }
                   else
                   {
                       // Nothing to do
                       return false;
                   }
               }
           }

           for (const auto& a : args)
           {
               prefix += " " + a;
           }

           for (auto paramCandidate : paramCandidates)
           {
               candidates.push_back(prefix + " " + paramCandidate);
           }

       }

       return true;
   }

   const std::list<std::string> ConsoleApp::getAutocompletePosibilities(const char* cmd) const
   {
       std::list<std::string> posibilities;

       std::string cmdToAutocomplete = cmd;
       for (const auto& c : mCommands)
       {
           c.second->getAutocompleteCandidates(cmdToAutocomplete, posibilities);
       }

       return posibilities;
   }

   const std::string ConsoleApp::getAutocomplete(std::list<std::string>& candidates) const
   {
       std::string autoCompleteSubstring;

       if (candidates.size() == 1)
       {
           autoCompleteSubstring = *candidates.begin();
       }
       else if (candidates.size() >= 2)
       {
           size_t matching_count = std::numeric_limits<size_t>::max();
           auto firstCandidate = *candidates.begin();
           for (auto candidate = std::next(candidates.begin()); candidate != candidates.end(); candidate++)
           {
               size_t c = getMatchingChars(*candidate, firstCandidate);
               if (c < matching_count)
               {
                   matching_count = c;
               }
           }
           autoCompleteSubstring = firstCandidate.substr(0, matching_count);
       }

       return autoCompleteSubstring;
   }

   void ConsoleApp::CliCommand::execute(std::list<std::string>& argList, std::ostream& out)
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
            switch (processGlobalOption(arg, out))
            {
            case Continue: continue;
            case Terminate: return;
            default:;
            }

            auto argDescIt = mOptions.find(arg);
            if (argDescIt == mOptions.end())
            {
               //printf("Invalid option '%s' for command '%s'. For more details use '%s help' or '%s %s -h'.\n", arg.c_str(), mCommandName.c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());

               out << "Invalid option '" << arg << "' for command '" << mCommandName << "'. For more details use '" << OwnName << " help' or '" << OwnName << " " << mCommandName << " -h'.\n";
               return;
            }

            if(argDescIt->second.hasValue)
            {
               if (argList.empty())
               {
                  //printf("Missing value '%s' for option '%s'. For more details use '%s help' or '%s %s -h'.\n", argDescIt->second.valueName.c_str(), argDescIt->second.name.c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());
                  out << "Missing value '" << argDescIt->second.valueName << "' for option '" << argDescIt->second.name << "'. For more details use '" << OwnName << " help' or '" << OwnName << " " << mCommandName << " -h'.\n";
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
            //printf("Missing parameter '%s'. For more details use '%s help' or '%s %s -h'.\n", mParameters[i].name.c_str(), OwnName.c_str(), OwnName.c_str(), mCommandName.c_str());
            out << "Missing parameter '" << mParameters[i].name << ". For more details use '" << OwnName << " help' or '" << OwnName << " " << mCommandName << " -h'.\n";
            return;
         }

         commandArgs.addParameter(mParameters[i].name, consoleParameters[i]);
      }

      // Execute command
      mExec(out, commandArgs);
   }

   ConsoleApp::CliCommand::Action ConsoleApp::CliCommand::processGlobalOption(std::string& option, std::ostream& out)
   {
      if(option == "-h")
      {
         if (!getSummary().empty())
         {
            //printf("%s\n\n", getSummary().c_str());
            out << getSummary() << "\n\n";
         }

         //printf("NAME\n   %s\n\n", getName().c_str());
         out << "NAME\n   " << getName() << "\n\n";
         //printf("SYNTAX\n   %s\n\n", getSyntax().c_str());
         out << "SYNTAX\n   " << getSyntax() << "\n\n";


         if (!getDescription().empty())
         {
            //printf("%s\n\n", getDescription().c_str());
            out << getDescription() << "\n\n";
         }

         return Terminate;
      }

      return Ignore;
   }

} // namespace qcli
