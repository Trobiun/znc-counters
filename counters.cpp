#include <vector>
#include <string>
#include <ctime>
#include <functional>
#include <znc/main.h>
#include <znc/Modules.h>
#include <znc/IRCNetwork.h>
#include <znc/Chan.h>
#include <znc/User.h>
#include "argparse.hpp"



//CONSTANTS DEFAULTS
const int DEFAULT_INITIAL = 0;
const int DEFAULT_STEP = 1;
const int DEFAULT_COOLDOWN = 0;
const int DEFAULT_DELAY = 0;
const std::string DEFAULT_MESSAGE = "{NAME} has value : {CURRENT_VALUE}";

class MyMap : public MCString {
private:

    MyMap() {

    }

    ~MyMap() {
    }

protected:


public:

    //CONSTRUCTORS AND DESCTRUTORS FOR SINGLETON

    static MyMap& getInstance() {
        static MyMap instance;
        return instance;
    }
    MyMap(MyMap const&) = delete;
    void operator=(MyMap const&) = delete;

};

class CCounter {
protected:
    //DATA MEMBERS
    //"constants" defined by constructor and can be changed by user with "set" command
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
    double m_time_chrono;

    //other variable
    std::time_t m_creation_datetime;


    //MEMBER FUNCTIONS

    /**
     * Set the previous value at current value and set last_change to now.
     * Should be called before changing current value.
     */
    void preChangeValue() {
        m_previous_value = m_current_value;
        //to improve : it's possible to send multiple messages in same time !
        time_t now = time(nullptr);
        double diffTime = difftime(now, m_last_change);
        m_time_chrono -= diffTime;
        if (m_time_chrono < 0) {
            m_time_chrono = m_cooldown;
        }
        //to avoid flood of messages when the difference between 'm_last_change' and 'now' is 0
        if (diffTime == 0) {
            m_time_chrono -= 1;
        }
        m_last_change = now;
    }

    /**
     * Change minimum and maximum values depending of current value.
     * Should be called after changing current valaue.
     */
    void postChangeValue() {
        if (m_current_value < m_minimum_value) {
            m_minimum_value = m_current_value;
        }
        if (m_current_value > m_maximum_value) {
            m_maximum_value = m_current_value;
        }
    }

    /**
     * Reset minimum, maximum, previous values to current value.
     */
    void resetValues() {
        m_maximum_value = m_minimum_value = m_previous_value = m_current_value;
    }

public:

    //CONSTRUCTORS & DESTRUCTOR

    CCounter(const CString& sName, const int initial = DEFAULT_INITIAL, const int step = DEFAULT_STEP,
            const int cooldown = DEFAULT_COOLDOWN, const int delay = DEFAULT_DELAY,
            const CString& sMessage = DEFAULT_MESSAGE) : m_sName(sName), m_initial(initial),
    m_step(step), m_cooldown(cooldown), m_delay(delay), m_sMessage(sMessage) {

        m_previous_value = m_current_value = initial;
        m_maximum_value = m_minimum_value = m_current_value;
        m_last_change = m_creation_datetime = time(nullptr);
        m_time_chrono = -1;
    }

    ~CCounter() {

    }


    //GETTERS

    CString getInfos(CUser* user) {
        return CString("Name : " + m_sName + "\nCreated at : " + getCreationTime(user)
                + "\nInitial : " + CString(m_initial) + "\nStep : " + CString(m_step)
                + "\nCooldown : " + CString(m_cooldown) + "\nDelay : " + CString(m_delay)
                + "\nMessage : " + m_sMessage + "\nCurrent : " + CString(m_current_value)
                + "\nPrevious : " + CString(m_previous_value) + "\nMinimum : "
                + CString(m_minimum_value) + "\nMaximum : " + CString(m_maximum_value)
                + "\nLast change : " + getLastChangeTime(user));
    }

    CTable getInfosTable(CUser* user) {
        CTable tableInfos = CTable();
        tableInfos.AddColumn("Attribute");
        tableInfos.AddColumn("Value");
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Name");
        tableInfos.SetCell("Value", m_sName);
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Created at");
        tableInfos.SetCell("Value", getCreationTime(user));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Initial");
        tableInfos.SetCell("Value", CString(m_initial));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Step");
        tableInfos.SetCell("Value", CString(m_step));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Cooldown");
        tableInfos.SetCell("Value", CString(m_cooldown));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Delay");
        tableInfos.SetCell("Value", CString(m_delay));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Message");
        tableInfos.SetCell("Value", m_sMessage);
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Current value");
        tableInfos.SetCell("Value", CString(m_current_value));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Previous value");
        tableInfos.SetCell("Value", CString(m_previous_value));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Minimum value");
        tableInfos.SetCell("Value", CString(m_minimum_value));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Maximum value");
        tableInfos.SetCell("Value", CString(m_maximum_value));
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Last change");
        tableInfos.SetCell("Value", getLastChangeTime(user));
        return tableInfos;
    }

    CString getName() {
        return m_sName;
    }

    int getDelay() {
        return m_delay;
    }

    CString getCreationTime(CUser* user) {
        return CUtils::FormatTime(m_creation_datetime, "%Y/%m/%d %H:%M:%S", user->GetTimezone());
    }

    CString getLastChangeTime(CUser* user) {
        return CUtils::FormatTime(m_last_change, "%Y/%m/%d %H:%M:%S", user->GetTimezone());
    }

    //to improve ?

    bool hasActiveCooldown() {
        return m_time_chrono < m_cooldown && m_cooldown != 0;
    }

    CString getCurrentValue() {
        return CString(m_current_value);
    }

    int getPreviousValue() {
        return m_previous_value;
    }

    int getMinimumValue() {
        return m_previous_value;
    }

    int getMaximumValue() {
        return m_maximum_value;
    }

    double getDiffTime() {
        return m_time_chrono;
    }

    CString getNamedFormat() {
        MyMap::getInstance().at("NAME") = m_sName;
        MyMap::getInstance().at("INITIAL") = CString(m_initial);
        MyMap::getInstance().at("STEP") = CString(m_step);
        MyMap::getInstance().at("COOLDOWN") = CString(m_cooldown);
        MyMap::getInstance().at("DELAY") = CString(m_delay);
        MyMap::getInstance().at("PREVIOUS_VALUE") = CString(m_previous_value);
        MyMap::getInstance().at("CURRENT_VALUE") = CString(m_current_value);
        MyMap::getInstance().at("MINIMUM_VALUE") = CString(m_minimum_value);
        MyMap::getInstance().at("MAXIMUM_VALUE") = CString(m_maximum_value);
        return CString::NamedFormat(m_sMessage, MyMap::getInstance());
    }


    //SETTERS

    void setName(const CString sName) {
        m_sName = sName;
    }

    void setInitial(const int initial) {
        m_initial = initial;
    }

    void setStep(const int step) {
        m_step = step;
    }

    void setCooldown(const int cooldown) {
        m_cooldown = cooldown;
    }

    void setDelay(const int delay) {
        m_delay = delay;
    }

    void setMessage(const CString& sMessage) {
        m_sMessage = sMessage;
    }

    /**
     * Reset the counter at resetValue.
     * @param resetValue the value that counter will take.
     */
    void reset(const int resetValue) {
        preChangeValue();
        m_current_value = resetValue;
        resetValues();
    }

    /**
     * Reset the counter at the initial value.
     */
    void resetDefault() {
        reset(m_initial);
    }

    void increment(int step) {
        preChangeValue();
        m_current_value += step;
        postChangeValue();
    }

    void incrementDefault() {
        increment(m_step);
    }

    void decrement(const int step) {
        preChangeValue();
        m_current_value -= step;
        postChangeValue();
    }

    void decrementDefault() {
        decrement(m_step);
    }

};

#ifdef HAVE_PTHREAD

class CCounterJob : public CModuleJob {
protected:

    CCounter m_counter;


public:

    CCounterJob(CModule* pModule, CCounter counter) : CModuleJob(pModule, "counters",
    "Send message for counter on channel after a delay"), m_counter(counter) {

    }

    virtual ~CCounterJob() override {
        //        if (wasCancelled()) {
        //            GetModule()->PutModule("Counter job cancelled");
        //        }
        //        else {
        //            GetModule()->PutModule("Counter job destroyed");
        //        }
    }

    virtual void runThread() override {
        int delay = m_counter.getDelay();
        for (int i = 0; i < delay; i++) {
            if (wasCancelled()) {
                return;
            }
            sleep(1);
        }
    }

    virtual void runMain() override {
        CString formattedMessage = m_counter.getNamedFormat();
        CIRCNetwork* network = GetModule()->GetNetwork();
        std::vector<CChan*> channels = network->GetChans();
        for (CChan* channel : channels) {
            GetModule()->PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
        }
    }

};
#endif

class CCounterListener {
};

class CCountersMod : public CModule {
protected:
    //DATA MEMBERS
    std::map<CString, CCounter> m_counters;
    /**
     * map with keys as couple (nickname,listener_name) and value as counter_name
     */
    std::map<std::pair<CString, CString>, CString> m_listeners;
    ArgumentParser m_parserCreate;


    //FUNCTIONS

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
     * Check if a string is empty.
     * @param text
     * @param defaultText
     * @return if text is empty, returns defaultText, else returns text.
     */
    CString checkStringValue(const CString text, CString defaultText) {
        return text.empty() ? defaultText : text;
    }

    /**
     * Create a counter.
     * @param sName the name of the counter
     * @param initial the initial value of counter
     * @param step the step by default to increment of decrement
     * @param cooldown the cooldown between 2 increment or decrement
     * @param delay the delay to write message on channel
     * @param sMessage the message to write on channel when current value change
     */
    void createCounter(const CString& sName, const int initial,
            const int step, const int cooldown, const int delay, const CString& sMessage) {
        if (!m_counters.count(sName)) {
            CCounter addCounter = CCounter(sName, initial, step, cooldown, delay, sMessage);
            auto created = m_counters.insert(std::pair<CString, CCounter>(sName, addCounter));
            if (created.second) {
                PutModule("Counter '" + addCounter.getName() + "' created.");
            }
        }
        else {
            PutModule("Counter '" + sName + "' already exists.");
        }
    }

    void createListener(const CString sName, const CString sNickname, const CString sListenerName) {
        auto created = m_listeners.insert(std::make_pair(std::make_pair(sNickname, sListenerName), sName));
        if (created.second) {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname +
                    "' and counter '" + sName + "' created.");
        }
    }

    void deleteListener(const CString sNickname, const CString sListenerName) {
        std::map<std::pair<CString, CString>, CString>::size_type removed = m_listeners.erase(std::make_pair(sNickname, sListenerName));
        if (removed) {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname + "' deleted.");
        }
        else {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname + "' not found.");
        }
    }

    //MODULE'S HOOKS

    virtual EModRet OnChanMsg(CNick& Nick, CChan& Channel, CString& sMessage) override {
        CString sListenerName = sMessage.Token(0);
        CString sNickname = Nick.GetNick();
        try {
            CString sCounterName = m_listeners.at(std::make_pair(sNickname, sListenerName));
            if (!sCounterName.empty()) {
                try {
                    CCounter counter = m_counters.at(sCounterName);
                    CString sCommand = sMessage.Token(1);
                    CString sArgs = sMessage.Token(2, true);
                    OnModCommand(sCommand + " " + counter.getName() + " " + sArgs);
                }
                catch (const std::out_of_range oor) {
                    PutModule("Counter '" + sCounterName + "' not found.");
                }
            }
        }
        catch (const std::out_of_range oor) {

        }
        return CONTINUE;
    }


    //MODULE'S COMMANDS
    //COUNTERS COMMANDS

    /**
     * Module command to create counter, parse sCommand.
     * @param sCommand command written by user to parse
     */
    void createCounterCommand(const CString& sCommand) {
        VCString vsArgs;
        sCommand.Split(" ", vsArgs, false, "\"", "\"", true, true);
        std::vector<std::string> args = std::vector<std::string>();

        for (const CString& arg : vsArgs) {
            if (!arg.empty()) {
                args.push_back((std::string)arg);
            }
        }
        try {
            m_parserCreate.parse(args);
        }
        catch (std::invalid_argument ex) {
            PutModule("Error invalid argument : " + CString(ex.what()));
            return;
        }
        //        catch (std::bad_cast) {
        //            PutModule("error bad cast");
        //        }
        //retrieve all arguments as strings because i get std::bad_cast with other typenames like int
        CString sInitial = CString(m_parserCreate.retrieve<std::string>("initial"));
        CString sStepValue = CString(m_parserCreate.retrieve<std::string>("step"));
        CString sCooldownValue = CString(m_parserCreate.retrieve<std::string>("cooldown"));
        CString sDelayValue = CString(m_parserCreate.retrieve<std::string>("delay"));
        CString sMessage = CString(m_parserCreate.retrieve<std::string>("message"));
        CString sName = CString(m_parserCreate.retrieve<std::string>("name"));

        createCounter(checkStringValue(sName, "counter"), convertWithDefaultValue(sInitial, DEFAULT_INITIAL),
                convertWithDefaultValue(sStepValue, DEFAULT_STEP), convertWithDefaultValue(sCooldownValue, DEFAULT_COOLDOWN),
                convertWithDefaultValue(sDelayValue, DEFAULT_DELAY), checkStringValue(sMessage, DEFAULT_MESSAGE));
        m_parserCreate.clearVariables();
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
        std::map<CString, CCounter>::size_type erased = m_counters.erase(sName);
        if (erased) {
            PutModule("Counter '" + sName + "' deleted.");
        }
        else {
            PutModule("Counter " + sName + " not found.");
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
    void executeSimpleCommand(const CString& sCommand, std::function<void(CCounter&, int) > execute,
            std::function<void(CCounter&) > executeWithDefault) {
        CString sName = sCommand.Token(1);
        CString sStep = sCommand.Token(2);
        if (!sName.empty()) {
            try {
                CCounter& counter = m_counters.at(sName);
                if (sStep.empty()) {
                    executeWithDefault(counter);
                }
                else {
                    execute(counter, sStep.ToInt());
                }
                if (!counter.hasActiveCooldown()) {
#ifdef HAVE_PTHREAD
                    AddJob(new CCounterJob(this, counter));
#else
                    CString formattedMessage = counter.getNamedFormat();
                    PutModule(formattedMessage);
                    CIRCNetwork *network = GetNetwork();
                    std::vector<CChan*> channels = network->GetChans();
                    for (CChan* channel : channels) {
                        PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
                    }
#endif
                }
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter " + sName + " not found.");
            }
        }
    }

    void resetCounterCommand(const CString& sCommand) {
        executeSimpleCommand(sCommand, &CCounter::reset, &CCounter::resetDefault);
    }

    void incrementCounterCommand(const CString& sCommand) {
        executeSimpleCommand(sCommand, &CCounter::increment, &CCounter::incrementDefault);
    }

    void decrementCounterCommand(const CString& sCommand) {
        executeSimpleCommand(sCommand, &CCounter::decrement, &CCounter::decrementDefault);
    }

    void printCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = m_counters.at(sName);
            CString formattedMessage = counter.getNamedFormat();
            CIRCNetwork* network = GetNetwork();
            std::vector<CChan*> channels = network->GetChans();
            for (CChan* channel : channels) {
                PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
            }
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter '" + sName + "' not found.");
        }
    }

    void infoCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = m_counters.at(sName);
            PutModule(counter.getInfosTable(GetUser()));
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter " + sName + " not found.");
        }
    }

    //TODO : to improve or change because it's possible to change counter's name
    //but this doesn't change used name in the map m_counters

    void setPropertyCounterCommand(const CString& sCommand) {
        VCString vsArgs;
        sCommand.Split(" ", vsArgs, false, "\"", "\"", true, true);
        try {
            CString sName = vsArgs.at(1);
            CString sProperty = vsArgs.at(2);
            CString sValue = vsArgs.at(3);
            try {
                CCounter& counter = m_counters.at(sName);
                if (sProperty.Equals("NAME"))
                    counter.setName(sValue);
                else if (sProperty.Equals("INITIAL"))
                    counter.setInitial(convertWithDefaultValue(sValue, 0));
                else if (sProperty.Equals("STEP"))
                    counter.setStep(convertWithDefaultValue(sValue, 1));
                else if (sProperty.Equals("COOLDOWN"))
                    counter.setCooldown(convertWithDefaultValue(sValue, 0));
                else if (sProperty.Equals("DELAY"))
                    counter.setDelay(convertWithDefaultValue(sValue, 0));
                else if (sProperty.Equals("MESSAGE"))
                    counter.setMessage(sValue);
                else
                    PutModule("Incorrect property ! Possibles properties are : name, "
                        "initial, step, cooldown, delay and message.");

                PutModule("Property '" + sProperty + "' of counter '" + sName +
                        "' changed to '" + sValue + "' value.");
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter '" + sName + "' not found.");
            }
        }
        catch (const std::out_of_range oor) {
            PutModule("Too few arguments.");
        }
    }

    void listCountersCommand(const CString& sCommand) {
        CString sCounters = "Your counters : ";
        for (std::map<CString, CCounter>::const_iterator it = m_counters.cbegin(); it != m_counters.cend(); ++it) {
            sCounters.append(it->first);
            if (it != std::prev(m_counters.cend())) {
                sCounters.append(", ");
            }
        }
        PutModule(sCounters);
    }


    //LISTENERS COMMANDS

    void createListenerCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        if (m_counters.count(sName)) {
            CString sNickname = sCommand.Token(2);
            CString sListenerName = sCommand.Token(3);
            if (sNickname.empty()) {
                sNickname = GetUser()->GetNick();
            }
            if (sListenerName.empty()) {
                sListenerName = "!" + sName;
            }
            createListener(sName, sNickname, sListenerName);
        }
        else {
            PutModule("Counter '" + sName + "' not found.");
        }
    }

    void deleteListenerCommand(const CString& sCommand) {
        CString sNickname = sCommand.Token(1);
        CString sListenerName = sCommand.Token(2);
        deleteListener(sNickname, sListenerName);
    }

    void listListenersCommand(const CString& sCommand) {
        CString sListeners = "Your listeners : ";
        for (std::map<std::pair<CString, CString>, CString>::const_iterator it = m_listeners.cbegin(); it != m_listeners.cend(); ++it) {
            sListeners.append(it->first.second + " for user " + it->first.first + " and counter " + it->second);
            if (it != std::prev(m_listeners.cend())) {
                sListeners.append(", ");
            }
        }
        PutModule(sListeners);
    }


public:

    MODCONSTRUCTOR(CCountersMod) {
        //create ArgumentParser to parse arguments for the command that create a counter
        m_parserCreate = ArgumentParser();
        m_parserCreate.useExceptions(true);
        m_parserCreate.ignoreFirstArgument(true);
        m_parserCreate.addArgument("-i", "--initial", 1, true);
        m_parserCreate.addArgument("-s", "--step", 1, true);
        m_parserCreate.addArgument("-c", "--cooldown", 1, true);
        m_parserCreate.addArgument("-d", "--delay", 1, true);
        m_parserCreate.addArgument("-m", "--message", 1, true);
        m_parserCreate.addFinalArgument("name", 1, false);
        MyMap::getInstance().insert(std::make_pair<CString, CString>("NAME", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("INITIAL", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("STEP", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("COOLDOWN", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("DELAY", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("PREVIOUS_VALUE", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("CURRENT_VALUE", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("MINIMUM_VALUE", ""));
        MyMap::getInstance().insert(std::make_pair<CString, CString>("MAXIMUM_VALUE", ""));

        AddHelpCommand();
        //COMMAND FOR COUNTERS
        AddCommand("Create", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::createCounterCommand),
                "[--initial | -i <initial>] [--step | -s <step>] [--cooldown | -c <cooldown>]"
                "[--delay | -d <delay>] [--message | -m \"<message>\"] <name>",
                "Create a counter.");
        AddCommand("Delete", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::deleteCounterCommand),
                "<name>", "Delete <name> counter.");
        AddCommand("Reset", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::resetCounterCommand),
                "<name> [reset_value]", "Reset <name> counter.");
        AddCommand("Incr", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::incrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
        AddCommand("Decr", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::decrementCounterCommand),
                "<name> [step]", "Decrement <name> counter.");
        AddCommand("Info", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::infoCounterCommand),
                "<name>", "Show information of <name> counter.");
        AddCommand("Set", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::setPropertyCounterCommand),
                "<name> <property> <value>", "Set property <property> to <value> for counter <name>.");
        AddCommand("List", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::listCountersCommand),
                "", "List counters.");
        AddCommand("Print", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::printCounterCommand),
                "<name>", "Print message for <name> counter.");

        //COMMANDS FOR LISTENERS
        AddCommand("CreateListener", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::createListenerCommand),
                "<name> <nickname> <listener_name>", "Create a listener : alias that can be used "
                "on any IRC client (like Twitch).");
        AddCommand("DeleteListener", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::deleteListenerCommand),
                "<nickname> <listener_name>", "Delete a listener.");
        AddCommand("ListListeners", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::listListenersCommand),
                "", "List listeners.");
    }

    virtual bool OnLoad(const CString& sArgs, CString& sMessage) override {
        return true;
    }

    virtual ~CCountersMod() {

    }

};

NETWORKMODULEDEFS(CCountersMod, "Module to count things using commands")