#ifndef Header_qcore_ConsoleApp
#define Header_qcore_ConsoleApp

#include <map>
#include <functional>
#include <vector>
#include <list>
#include <set>

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

      typedef std::function<void(const CliArgs&)> CommandCb;

      class CliCommand
      {
         struct Option
         {
            std::string name;
            std::string valueName;
            bool hasValue;
         };

         enum Action
         {
            Ignore = 0,
            Continue,
            Terminate
         };

      private:
         Action processGlobalOption(std::string& option);

         std::string mCommandName;
         CommandCb mExec;
         std::string mSyntax;
         std::string mSummary;
         std::string mDescription;

         std::map<std::string, Option> mOptions;
         std::vector<std::string> mParameters;

      public:
         CliCommand(CommandCb exec, const std::string& syntax);

         CliCommand& setName(std::string name) { mCommandName = name; return *this; }
         CliCommand& setSummary(std::string summary) { this->mSummary = summary; return *this; }
         CliCommand& setDescription(std::string description) { this->mDescription = description; return *this; }
         CliCommand& addParameter(std::string name) { mParameters.push_back(name); return *this; }
         CliCommand& addOption(std::string name, std::string valueName = "")  { mOptions[name] = { name, valueName, !valueName.empty() }; return *this; }

         void execute(std::list<std::string>& argList);

         const std::string& getName() const { return mCommandName; }
         const std::string& getSyntax() const { return mSyntax; }
         const std::string& getSummary() const { return mSummary; }
         const std::string& getDescription() const { return mDescription; }
      };

   private:

      static std::string OwnName;
      std::map<std::string, CliCommand*> mCommands;
      std::map<std::string, std::list<CliCommand*>> mOrderedCommands;

   public:

      ConsoleApp();
      ~ConsoleApp();

      // syntax example: command subcommand1 subcommand2 <param1> <param2> <param3> -option1 <optionValue1> -option2 <optionValue2> -option3
      CliCommand& addCommand(CommandCb exec, const std::string& syntax, const std::string& section = "");
      int executeOnce(int argc, char** argv);
      int execute(int argc, char** argv);
      void printHelp();
   };
}

#endif // Header_qcore_ConsoleApp
