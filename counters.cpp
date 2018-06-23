#include <vector>
#include <string>
#include <ctime>
#include <functional>
#include <znc/znc.h>
#include <znc/IRCNetwork.h>
#include <znc/Chan.h>
#include "argparse.hpp"

//class Property {
//protected:
//    enum class EnumProperty {
//        NAME,
//        INITIAL,
//        STEP,
//        COOLDOWN,
//        DELAY,
//        MESSAGE
//    } lol;
//    
//    
//public:
//    
//    Property(EnumProperty ep) {
//        
//    }
//
//};

const int DEFAULT_INITIAL = 0;
const int DEFAULT_STEP = 1;
const int DEFAULT_COOLDOWN = 0;
const int DEFAULT_DELAY = 0;
const std::string DEFAULT_MESSAGE = "{NAME} has value : {CURRENT_VALUE}";

class CCounter {
protected:
    //STATIC VARIABLES OR TESTS
    MCString m_formatMap;
    
    
    //DATA MEMBERS
    //"constants" defined by constructor
    CString m_sName;
    int m_initial;
    int m_step;
    int m_cooldown; /**< Cooldown between 2 messages when value change. */
    int m_delay; /**< Delay to send message when value change. */
    CString m_sMessage; /**< The message to send when value change. */
    
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
    //STATIC FUNCTION TO CREATE MAP OF FORMAT
    static void initializeFormatMap(CCounter& counter) {
//        formatMap.insert(std::pair<CString,CString>("NAME"),counter.);
    }
    
    //CONSTRUCTOR & DESTRUCTOR
    CCounter(const CString& sName, const int initial = DEFAULT_INITIAL, const int step = DEFAULT_STEP,
            const int cooldown = DEFAULT_COOLDOWN, const int delay = DEFAULT_DELAY,
            const CString& sMessage = DEFAULT_MESSAGE) {
        this->m_sName = sName;
        this->m_initial = initial;
        this->m_step = step;
        this->m_cooldown = cooldown;
        this->m_delay = delay;
        this->m_sMessage  = sMessage;
        
        this->m_previous_value = this->m_current_value = initial;
        this->m_maximum_value = this->m_minimum_value = this->m_current_value;
        this->m_last_change = this->m_creation_datetime = time(nullptr);
        
//        this->m_formatMap = MCString();
//        this->m_formatMap.insert(std::pair<CString,CString>("NAME",this->m_sName));
//        this->m_formatMap.insert(std::pair<CString,CString>("INITIAL",this->m_initial));
//        this->m_formatMap.insert(std::pair<CString,CString>("STEP",this->m_step));
//        this->m_formatMap.insert(std::pair<CString,CString>("COOLDOWN",this->m_cooldown));
//        this->m_formatMap.insert(std::pair<CString,CString>("DELAY",this->m_delay));
//        this->m_formatMap.insert(std::pair<CString,CString>("PREVIOUS_VALUE",this->m_previous_value));
//        this->m_formatMap.insert(std::pair<CString,CString>("CURRENT_VALUE",this->m_current_value));
//        this->m_formatMap.insert(std::pair<CString,CString>("MINIMUM_VALUE",this->m_minimum_value));
//        this->m_formatMap.insert(std::pair<CString,CString>("MAXIMUM_VALUE",this->m_maximum_value));
        
    }
    
    ~CCounter() {

    }
    
    
    //GETTERS
    CString getInfos() {
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
    
    bool hasActiveCooldown() {
        double diffTime = difftime(time(nullptr),this->m_last_change);
        if (diffTime < this->m_cooldown || this->m_cooldown == 0) {
            return false;
        }
        return true;
    }
    
    CString getCurrentValue() {
        return CString(this->m_current_value);
    }
    
    int getPreviousValue() {
        return this->m_previous_value;
    }
    
    int getMinimumValue() {
        return this->m_previous_value;
    }
    
    int getMaximumValue() {
        return this->m_maximum_value;
    }
    
    CString sendMessage() {
        MCString map = MCString();
        map.insert(std::pair<CString,CString>("NAME",this->m_sName));
        map.insert(std::pair<CString,CString>("INITIAL",this->m_initial));
        map.insert(std::pair<CString,CString>("STEP",this->m_step));
        map.insert(std::pair<CString,CString>("COOLDOWN",this->m_cooldown));
        map.insert(std::pair<CString,CString>("DELAY",this->m_delay));
        map.insert(std::pair<CString,CString>("PREVIOUS_VALUE",this->m_previous_value));
        map.insert(std::pair<CString,CString>("CURRENT_VALUE",this->m_current_value));
        map.insert(std::pair<CString,CString>("MINIMUM_VALUE",this->m_minimum_value));
        map.insert(std::pair<CString,CString>("MAXIMUM_VALUE",this->m_maximum_value));
        return CString::NamedFormat(this->m_sMessage,map);
    }
    
    
    //SETTERS
    void setName(const CString sName) {
        this->m_sName = sName;
    }
    
    void setInitial(const int initial) {
        this->m_initial = initial;
    }
    
    void setStep(const int step) {
        this->m_step = step;
    }
    
    void setCooldown(const int cooldown) {
        this->m_cooldown = cooldown;
    }
    
    void setDelay(const int delay) {
        this->m_delay = delay;
    }
    
    void setMessage(const CString& sMessage) {
        this->m_sMessage = sMessage;
    }
    
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
    
    void increment(int step) {
        this->preChangeValue();
        this->m_current_value += step;
        this->postChangeValue();
    }
    
    void incrementDefault() {
        this->increment(this->m_step);
    }
    
    void decrement(const int step) {
        this->preChangeValue();
        this->m_current_value -= step;
        this->postChangeValue();
    }
    
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
        return text.empty() ? defaultText : text;
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
    void createCounter(const CString& sName, const int initValue = DEFAULT_INITIAL,
    const int step = DEFAULT_STEP, const int cooldown = DEFAULT_COOLDOWN,
    const int delay = DEFAULT_DELAY, const CString& sMessage = DEFAULT_MESSAGE) {
        CCounter addCounter = CCounter(sName, initValue, step, cooldown, delay, sMessage);
        this->m_counters.insert(std::pair<CString,CCounter>(sName, addCounter));
        PutModule("Compteur ajouté : " + addCounter.getName());
        PutModule("Compteur créé le : " + addCounter.getCreationTime());
        PutModule(addCounter.getInfos());
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
            if (!arg.empty()) {
                PutModule("Argument : " + arg);
                args.push_back((std::string)arg);
            }
            else {
                PutModule("Argument vide.");
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
        
        this->createCounter(checkStringValue(sName,"counter"),convertWithDefaultValue(sInitValue,DEFAULT_INITIAL),
                convertWithDefaultValue(sStepValue,DEFAULT_STEP),convertWithDefaultValue(sCooldownValue,DEFAULT_COOLDOWN),
                convertWithDefaultValue(sDelayValue,DEFAULT_DELAY),checkStringValue(sMessage,DEFAULT_MESSAGE));
        this->m_parserCreate.clearVariables();
//        MCString msRet;
//        CString::size_type tokensNb4 = sCommand.OptionSplit(msRet);
//        PutModule("Commande séparée en : " + CString(tokensNb4) + " chaines avec OptionSplit.");
//        MCString::iterator itMap;
//        for (itMap = msRet.begin(); itMap != msRet.end(); itMap++) {
//            PutModule("clé : " + itMap->first + " , valeur : " + itMap->second);
//        }
        
    }
    
    void deleteCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        if (!sName.empty()) {
            this->m_counters.erase(sName);
        }
    }
    
    /**
     * Execute a simple command (with the name of counter, and an optional second
     * value) for a counter.\n
     * If second value is specified, execute first function as argument with value.\n
     * If second value is not specified, execute function that doesn't require value
     * (use default value from the counter for this command).
     * @param sCommand command written by user
     * @param execute function to execute with value
     * @param executeWithDefault function to execute without value
     */
    void executeSimpleCommand(const CString& sCommand, std::function<void(CCounter&,int)> execute,
    std::function<void(CCounter&)> executeWithDefault) {
        CString sName = sCommand.Token(1);
        CString sStep = sCommand.Token(2);
        if (!sName.empty()) {
            try {
                CCounter& counter = this->m_counters.at(sName);
                PutModule("Ancienne valeur : " + CString(counter.getCurrentValue()));
                if (sStep.empty()) {
                    PutModule("Excution par défaut.");
                    executeWithDefault(counter);
                }
                else {
                    PutModule("Exécution de : " + sStep);
                    execute(counter, sStep.ToInt());
                }
                PutModule("Valeur courante : " + CString(counter.getCurrentValue()));
                if (!counter.hasActiveCooldown()) {
                    CString formattedMessage = counter.sendMessage();
                    PutModule(formattedMessage);
                    CIRCNetwork *network = GetNetwork();
                    std::vector<CChan*> channels = network->GetChans();
                    for (CChan* channel : channels) {
                        PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
                    }
                }
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter not found.");
            }
        }
    }
    
    void resetCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::reset,&CCounter::resetDefault);
    }
    
    void incrementCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::increment,&CCounter::incrementDefault);
    }
    
    void decrementCounterCommand(const CString& sCommand) {
        this->executeSimpleCommand(sCommand,&CCounter::decrement,&CCounter::decrementDefault);
    }
    
    void infoCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = this->m_counters.at(sName);
            PutModule(counter.getInfos());
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter not found.");
        }
    }
    
    void setPropertyCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        CString sProperty = sCommand.Token(2);
        CString sValue = sCommand.Token(3);
        try {
            CCounter& counter = this->m_counters.at(sName);
            if (!sProperty.empty() && !sValue.empty()) {
                if (sProperty.Equals("NAME"))
                    counter.setName(sValue);
                else if (sProperty.Equals("INITIAL"))
                    counter.setInitial(convertWithDefaultValue(sValue,0));
                else if (sProperty.Equals("STEP"))
                    counter.setStep(convertWithDefaultValue(sValue,1));
                else if (sProperty.Equals("COOLDOWN"))
                    counter.setCooldown(convertWithDefaultValue(sValue,0));
                else if (sProperty.Equals("DELAY"))
                    counter.setDelay(convertWithDefaultValue(sValue,0));
                else if (sProperty.Equals("MESSAGE"))
                    counter.setMessage(sValue);
                else
                    PutModule("Incorrect property !");
            }
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter not found.");
        }
    }
    
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
//        AddCommand("+", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::incrementCounterCommand),
//                "<name> [step]", "Increment <name> counter.");
        AddCommand("decr", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::decrementCounterCommand),
                "<name> [step]","Decrement <name> counter.");
//        AddCommand("-", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::decrementCounterCommand),
//                "<name> [step]","Decrement <name> counter.");
        AddCommand("info", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::infoCounterCommand),
                "<name>","Show information of <name> counter.");
        AddCommand("set", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::listCountersCommand),
                "<name> <property> <value>","Set property <property> to <value> for counter <name>.");
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