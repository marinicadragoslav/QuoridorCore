#ifndef Header_qcore_ConsoleApp
#define Header_qcore_ConsoleApp

#include <map>
#include <functional>
#include <vector>
#include <list>
#include <set>
#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace qcli
{
   class ConsoleApp
   {
   public:

      class CliArgs
      {
      public:
         void addOption(const std::string& name) { mOptions.insert(name); }
         void addParameter(const std::string& name, const std::string& value) { mArgs[name] = value; }

         bool isSet(const std::string& name) const { return mOptions.find(name) != mOptions.end(); }
         std::string getValue(const std::string& name) const { auto it = mArgs.find(name); return it != mArgs.end() ? it->second : ""; }

      private:
         std::map<std::string, std::string> mArgs;
         std::set<std::string> mOptions;
      };

      typedef std::function<void(std::ostream& out, const CliArgs&)> CommandCb;

      class CliCommand
      {
         struct Option
         {
            std::string name;
            std::string valueName;
            bool hasValue;
         };

         class Parameter
         {
         public:
             typedef std::function<std::list<std::string>(const std::string& cmd)> AutocompleteCb;

             std::string name;
             AutocompleteCb autocompleteCb;
         };

         enum Action
         {
            Ignore = 0,
            Continue,
            Terminate
         };

      private:
         Action processGlobalOption(std::string& option, std::ostream& out);

         bool canAutocomplete(std::string& cmd) const;
         bool getParamsAutocomplete(const std::string& cmd, std::list<std::string>& candidates) const;

         std::string mCommandName;
         CommandCb mExec;
         std::string mSyntax;
         std::string mSummary;
         std::string mDescription;

         std::map<std::string, Option> mOptions;
         std::vector<Parameter> mParameters;

      public:
         CliCommand(CommandCb exec, const std::string& syntax);

         CliCommand& setName(std::string name) { mCommandName = name; return *this; }
         CliCommand& setSummary(std::string summary) { this->mSummary = summary; return *this; }
         CliCommand& setDescription(std::string description) { this->mDescription = description; return *this; }
         CliCommand& addParameter(std::string name, Parameter::AutocompleteCb autocompleteCb = nullptr) { mParameters.push_back({name, autocompleteCb}); return *this; }
         CliCommand& registerParameterAutocompleteCb(std::string name, Parameter::AutocompleteCb autocompleteCb) 
         {
             auto param = std::find_if( mParameters.begin(), mParameters.end(), [&name](const auto& x) { return x.name == name;});
             if (param == mParameters.end())
             {
                 throw "Parameter not found";
             }

             if(param->autocompleteCb)
               throw "Callback already registred";

             param->autocompleteCb = autocompleteCb; 
             return *this; 
         }
         CliCommand& addOption(std::string name, std::string valueName = "")  { mOptions[name] = { name, valueName, !valueName.empty() }; return *this; }

         void execute(std::list<std::string>& argList, std::ostream& out);

         const std::string& getName() const { return mCommandName; }
         const std::string& getSyntax() const { return mSyntax; }
         const std::string& getSummary() const { return mSummary; }
         const std::string& getDescription() const { return mDescription; }
         bool getAutocompleteCandidates(std::string& cmd, std::list<std::string>& candidates) const;
      };

   private:

      static std::string OwnName;
      std::map<std::string, CliCommand*> mCommands;
      std::map<std::string, std::list<CliCommand*>> mOrderedCommands;
      std::ostream& mOut;
      std::deque<std::string> mHistory;

   public:

       ConsoleApp(std::ostream& out);
      ~ConsoleApp();

      // syntax example: command subcommand1 subcommand2 <param1> <param2> <param3> -option1 <optionValue1> -option2 <optionValue2> -option3
      CliCommand& addCommand(CommandCb exec, const std::string& syntax, const std::string& section = "");
      CliCommand& RegisterParameterAutocomplete(const std::string);
      int executeOnce(int argc, char** argv);
      int runCli(int argc, char** argv);
      int executeCommand(const char* cmd);
      void printHelp();
      const std::string getAutocomplete(std::list<std::string>& candidates) const;
      const std::list<std::string> getAutocompletePosibilities(const char* cmd) const;
      std::string getResponse() { std::ostringstream buf; buf << mOut.rdbuf(); return buf.str(); };

   };
}

#endif // Header_qcore_ConsoleApp
