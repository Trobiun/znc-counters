#include <vector>
#include <string>
#include <ctime>
#include <functional>
#include <znc/znc.h>
#include <znc/IRCNetwork.h>
#include <znc/Chan.h>
#include "argparse.hpp"


class CCounter {
protected:
    //DATA MEMBERS
    //constants defined by constructor
    CString m_sName;
    int m_initial;
    int m_step;
    /**
     * Cooldown between 2 messages when value change.
     */
    int m_cooldown;
    /**
     * Delay to send message when value change.
     */
    int m_delay;
    /**
     * The message to send when value change.
     */
    CString m_sMessage;
    
    //values that can change
    int m_current_value;
    int m_previous_value;
    int m_minimum_value;
    int m_maximum_value;
    std::time_t m_last_change;
    
    //other variable
    std::time_t m_creation_datetime;
    
    
    //MEMBER FUNCTIONS
    /**
     * Set the previous value at current value and set last_change to now.
     * Must be called before changing current value.
     */
    void preChangeValue() {
        this->m_previous_value = this->m_current_value;
        this->m_last_change = time(nullptr);
    }
    
    /**
     * Change minimum and maximum values depending of current value.
     * Must be called after changing current valaue.
     */
    void postChangeValue() {
        if (this->m_current_value < this->m_minimum_value) {
            this->m_minimum_value = this->m_current_value;
        }
        if (this->m_current_value > this->m_maximum_value) {
            this->m_maximum_value = this->m_current_value;
        }
    }
    
    /**
     * Reset minimum, maximum, previous values to current value.
     */
    void resetValues() {
        this->m_maximum_value = this->m_minimum_value = this->m_previous_value = this->m_current_value;
    }
    
public:
    //CONSTRUCTOR & DESTRUCTOR
    CCounter(const CString& sName, const int initial = 0, const int step = 1,
            const int cooldown = 0, const int delay = 0, const CString& sMessage = "[NAME] has value : [CURRENT_VALUE]") {
        this->m_sName = sName;
        this->m_initial = initial;
        this->m_step = step;
        this->m_cooldown = cooldown;
        this->m_delay = delay;
        this->m_sMessage  = sMessage;
        
        this->m_previous_value = this->m_current_value = initial;
        this->m_maximum_value = this->m_minimum_value = this->m_current_value;
        this->m_creation_datetime = time(nullptr);
    }
    
    ~CCounter() {

    }
    
    
    //GETTERS
    CString prettyPrint() {
        return CString("Name : " + m_sName + "\nCreated at : " + getCreationTime()
                + "\nInitial : " + CString(m_initial) + "\nStep : " + CString(m_step)
                + "\nCooldown : " + CString(m_cooldown) + "\nDelay : " + CString(m_delay)
                + "\nMessage : " + m_sMessage + "\nCurrent : " + CString(m_current_value)
                + "\nPrevious : " + CString(m_previous_value) + "\nMinimum : "
                + CString(m_minimum_value) + "\nMaximum : " + CString(m_maximum_value)
                + "\nLast change : " + getLastChangeTime());
    }
    
    CString getName() {
        return this->m_sName;
    }
    
    CString getCreationTime() {
        char mbstr[20];
        if (std::strftime(mbstr, sizeof(mbstr), "%Y/%m/%d %H:%M:%S", std::localtime(&this->m_creation_datetime))) {
            return CString(mbstr);
        }
        return "Unknown date";
    }
    
    CString getLastChangeTime() {
        char mbstr[20];
        if (std::strftime(mbstr, sizeof(mbstr), "%Y/%m/%d %H:%M:%S", std::localtime(&this->m_last_change))) {
            return CString(mbstr);
        }
        return "Unknown date";
    }
    
    int getCurrentValue() {
        return this->m_current_value;
    }
    
    
    //SETTERS
    /**
     * Reset the counter at resetValue.
     * @param resetValue the value that counter will take.
     */
    void reset(const int resetValue) {
        this->preChangeValue();
        this->m_current_value = resetValue;
        this->resetValues();
    }
    
    /**
     * Reset the counter at the initial value.
     */
    void resetDefault() {
        this->reset(this->m_initial);
    }
    
    /**
     * Increment the counter by step value.
     * @param step
     */
    void increment(int step) {
        this->preChangeValue();
        this->m_current_value += step;
        this->postChangeValue();
    }
    
    /**
     * Increment the counter by default value of step for the counter.
     */
    void incrementDefault() {
        this->increment(this->m_step);
    }
    
    /**
     * Decrement the counter by step value.
     * @param step
     */
    void decrement(const int step) {
        this->preChangeValue();
        this->m_current_value -= step;
        this->postChangeValue();
    }
    
    /**
     * Decrement the counter by default value of step for the counter.
     */
    void decrementDefault() {
        this->decrement(this->m_step);
    }
    
};

class CCountersMod : public CModule {
protected:
    //DATA MEMBER
    std::map<CString,CCounter> m_counters;
    ArgumentParser m_parserCreate;
    
    
    /**
     * Casts a CString to another type (specially int). If the cast fails,
     * return the specified value.
     * @param text string to cast
     * @param value default value if fail
     * @return value represented by text, or value in fail
     */
    template<typename T>
    T convertWithDefaultValue(const CString text, const T value) {
        std::stringstream ss(text);
        T result;
        return ss >> result ? result : value;
    }
    
    /**
     * 
     * @param text
     * @param defaultText
     * @return 
     */
    CString checkStringValue(const CString text, CString defaultText) {
        return text.size() > 0 ? text : defaultText;
    }
    
    /**
     * Create a counter.
     * @param sName the name of the counter
     * @param initValue the initial value of counter
     * @param step the step by default to increment of decrement
     * @param cooldown the cooldown between 2 increment or decrement
     * @param delay the delay to write message on channel
     * @param sMessage the message to write on channel when current value change
     */
    void createCounter(const CString& sName, const int initValue = 0, const int step = 1,
    const int cooldown = 0, const int delay = 0, const CString& sMessage = "[NAME] has value : [CURRENT_VALUE]") {
        CCounter addCounter = CCounter(sName, initValue, step, cooldown, delay, sMessage);
        this->m_counters.insert(std::pair<CString,CCounter>(sName, addCounter));
        PutModule("Compteur ajouté : " + addCounter.getName());
        PutModule("Compteur créé le : " + addCounter.getCreationTime());
        PutModule(addCounter.prettyPrint());
    }
    
    /**
     * Module command to create counter, parse sCommand.
     * @param sCommand command written by user to parse
     */
    void createCounterCommand(const CString& sCommand) {
        PutModule("Commande écrite : " + sCommand);
        /*
        std::vector<CChan*> channels = GetNetwork()->GetChans();
        std::vector<CChan*>::iterator it;
        for (it = channels.begin(); it != channels.end(); ++it) {
            PutModule("PRIVMSG " + (*it)->GetName() + " :Message test 2");
            PutIRC("PRIVMSG " + (*it)->GetName() + " :Message test 3");
        }
        PutIRC("PRIVMSG test :message privé ?");
        */
//        GetNetwork()->PutIRC("Test");
//        GetNetwork()->PutIRC("PRIVMSG #tests :Message test 4");
        
        
        VCString vsArgs;
        CString::size_type numberTokens = sCommand.Split(" ", vsArgs, false, "\"", "\"", true, true);
        std::vector<std::string> args = std::vector<std::string>();

        for (const CString& arg : vsArgs) {
            if (arg.size() > 0) {
                PutModule("Argument : " + arg);
                args.push_back((std::string)arg);
            }
            else {
                PutModule("Argument vide");
            }
        }
        try {
            this->m_parserCreate.parse(args);
        }
        catch (std::invalid_argument ex) {
            PutModule("error invalid argument : " + CString(ex.what()));
            return;
        }
//        catch (std::bad_cast) {
//            PutModule("error bad cast");
//        }
        PutModule("Arguments parsés !");
        CString sInitValue = CString(this->m_parserCreate.retrieve<std::string>("initvalue"));
        CString sStepValue = CString(this->m_parserCreate.retrieve<std::string>("step"));
        CString sCooldownValue = CString(this->m_parserCreate.retrieve<std::string>("cooldown"));
        CString sDelayValue = CString(this->m_parserCreate.retrieve<std::string>("delay"));
        CString sMessage = CString(this->m_parserCreate.retrieve<std::string>("message"));
        CString sName = CString(this->m_parserCreate.retrieve<std::string>("name"));
        
        PutModule("Initial value : " + sInitValue);
        PutModule("Step value : " + sStepValue);
        PutModule("Cooldown value : " + sCooldownValue);
        PutModule("Delay value : " + sDelayValue);
        PutModule("Message : " + sMessage);
        PutModule("Name : " + sName);
        
        this->createCounter(checkStringValue(sName,"counter"),convertWithDefaultValue(sInitValue,0),convertWithDefaultValue(sStepValue,1),convertWithDefaultValue(sCooldownValue,0),
                convertWithDefaultValue(sDelayValue,0),checkStringValue(sMessage,"[NAME] has value : [CURRENT_VALUE]"));
        this->m_parserCreate.clearVariables();
//        MCString msRet;
//        CString::size_type tokensNb4 = sCommand.OptionSplit(msRet);
//        PutModule("Commande séparée en : " + CString(tokensNb4) + " chaines avec OptionSplit.");
//        MCString::iterator itMap;
//        for (itMap = msRet.begin(); itMap != msRet.end(); itMap++) {
//            PutModule("clé : " + itMap->first + " , valeur : " + itMap->second);
//        }
        
    }
    
    /**
     * Command of module to delete a counter.
     * @param sCommand command written by user
     */
    void deleteCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        if (sName.size() > 0) {
            this->m_counters.erase(sName);
        }
    }
    
    /**
     * Execute a simple command (with the name of counter, and an optional second
     * value) for a counter.\n
     * If second value is specified, execute first function as argument with value.\n
     * If second value is not specified, execute function that doesn't require value
     * (use default value from the counter for this command).
     * @param sCommand command passed written by user
     * @param execute function to execute with value
     * @param executeWithDefault function to execute without value
     */
    void executeSimpleCommand(const CString& sCommand, std::function<void(CCounter&,int)> execute,
    std::function<void(CCounter&)> executeWithDefault) {
        CString sName = sCommand.Token(1);
        CString sStep = sCommand.Token(2);
        if (sName.size() > 0) {
            try {
                CCounter& counter = this->m_counters.at(sName);
                PutModule("Ancienne valeur : " + CString(counter.getCurrentValue()));
                if (sStep.size() > 0) {
                    PutModule("Incrémentation de : " + sStep);
                    execute(counter, sStep.ToInt());
                }
                else {
                    PutModule("Incrémentation par défaut.");
                    executeWithDefault(counter);
                }
                PutModule("Valeur courante : " + CString(counter.getCurrentValue()));
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter not found.");
            }
        }
    }
    
    /**
     * Command of module to reset a counter.
     * @param sCommand command written by user
     */
    void resetCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::reset,&CCounter::resetDefault);
    }
    
    /**
     * Command of module to increment the counter.
     * @param sCommand command written by user
     */
    void incrementCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::increment,&CCounter::incrementDefault);
    }
    
    /**
     * Command of module to decrement a counter.
     * @param sCommand command written by user
     */
    void decrementCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::decrement,&CCounter::decrementDefault);
    }
    
    /**
     * Command of module to show informations of a counter.
     * @param sCommand command written by user
     */
    void infoCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = this->m_counters.at(sName);
            PutModule(counter.prettyPrint());
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter not found.");
        }
    }
    
    /**
     * Command of module to list actives counters.
     * @param sCommand command written by user (not used)
     */
    void listCountersCommand(const CString& sCommand) {
        CString sCounters = "Your counters : ";
        for (std::map<CString,CCounter>::const_iterator it = this->m_counters.cbegin(); it != this->m_counters.cend(); ++it) {
            sCounters.append((*it).first);
            if (it != std::prev(this->m_counters.cend())) {
                sCounters.append(", ");
            }
        }
        PutModule(sCounters);
    }
    
    
public:
    
    MODCONSTRUCTOR(CCountersMod) {
        this->m_parserCreate = ArgumentParser();
        this->m_parserCreate.useExceptions(true);
        this->m_parserCreate.ignoreFirstArgument(true);
        this->m_parserCreate.addArgument("-i", "--initvalue", 1, true);
        this->m_parserCreate.addArgument("-s", "--step", 1, true);
        this->m_parserCreate.addArgument("-c", "--cooldown", 1, true);
        this->m_parserCreate.addArgument("-d", "--delay", 1, true);
        this->m_parserCreate.addArgument("-m", "--message", 1, true);
        this->m_parserCreate.addFinalArgument("name", 1, false);
        AddHelpCommand();
        AddCommand("create", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::createCounterCommand),
                "[--initvalue | -i <initial>] [--step | -s <step>] [--cooldown | -c <cooldown>] [--delay | -d <delay>] [--message | -m \"<message>\"] <name>",
                "Create a counter.");
        AddCommand("delete", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::deleteCounterCommand),
                "<name>","Delete <name> counter.");
        AddCommand("reset", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::resetCounterCommand),
                "<name> [reset_value]","Reset <name> counter at reset_value or default value for counter.");
        AddCommand("incr", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::incrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
        AddCommand("+", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::incrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
        AddCommand("decr", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::decrementCounterCommand),
                "<name> [step]","Decrement <name> counter.");
        AddCommand("-", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::decrementCounterCommand),
                "<name> [step]","Decrement <name> counter.");
        AddCommand("info", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::infoCounterCommand),
                "<name>","Show information of <name> counter.");
        AddCommand("list", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::listCountersCommand),
                "","List counters.");
    }
    
    virtual bool OnLoad(const CString& sArgs, CString& sMessage) override {
        return true;
    }

    virtual ~CCountersMod() {
        
    }

};

NETWORKMODULEDEFS(CCountersMod, "Module to count things using commands")