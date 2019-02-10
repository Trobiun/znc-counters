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
const int DEFAULT_EACH = 1;
const std::string DEFAULT_MESSAGE = "{NAME} has value : {CURRENT_VALUE}";
const std::string DEFAULT_IMBRICATION_MESSAGE = "Counter {{SUB}NAME} went from {START} to {END}";
const std::string FORMAT_TIME = "%Y/%m/%d %H:%M:%S";

class MyMap : public MCString {
private:

    MyMap() {

    }

    ~MyMap() {
    }

protected:


public:

    //CONSTRUCTORS AND DESCTRUTORS FOR SINGLETON

    static MyMap& GetInstance() {
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
    int m_step; /**< Step to increment by default. */
    int m_cooldown; /**< Cooldown between 2 messages when value change. */
    int m_delay; /**< Delay to send message when value change. */
    int m_each; /**< Interval of current value between two messages. ex : every 10 increments or decrements send the message */
    CString m_sMessage; /**< The message to send when value change. */

    //values that can change
    int m_current_value;
    int m_previous_value;
    int m_previous_printed_value;
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
    void PreChangeValue() {
        m_previous_value = m_current_value;
        time_t now = time(nullptr);
        double diffTime = difftime(now, m_last_change);
        m_time_chrono -= diffTime;
        if (m_time_chrono < 0) {
            m_time_chrono = m_cooldown;
        }
        //to avoid possibility to flood of messages when the difference between 'm_last_change' and 'now' is 0
        if (diffTime == 0) {
            m_time_chrono -= 1;
        }
        m_last_change = now;
    }

    /**
     * Change minimum and maximum values depending of current value.
     * Should be called after changing current valaue.
     */
    void PostChangeValue() {
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
    void ResetValues() {
        m_maximum_value = m_minimum_value = m_previous_value = m_current_value;
    }

public:

    //CONSTRUCTORS & DESTRUCTOR

    CCounter(const CString& sName, const int initial = DEFAULT_INITIAL, const int step = DEFAULT_STEP,
            const int cooldown = DEFAULT_COOLDOWN, const int delay = DEFAULT_DELAY,
            const int each = DEFAULT_EACH, const CString& sMessage = DEFAULT_MESSAGE) :
    m_sName(sName), m_initial(initial), m_step(step), m_cooldown(cooldown),
    m_delay(delay), m_each(each), m_sMessage(sMessage) {

        m_previous_printed_value = m_previous_value = m_current_value = initial;
        m_maximum_value = m_minimum_value = m_current_value;
        m_last_change = m_creation_datetime = time(nullptr);
        m_time_chrono = -1;
    }

    ~CCounter() {

    }


    //GETTERS

    CString GetInfos(CUser* user) {
        return CString("Name : " + m_sName
                + "\nCreated at : " + GetCreationTime(user)
                + "\nInitial : " + CString(m_initial)
                + "\nStep : " + CString(m_step)
                + "\nCooldown : " + CString(m_cooldown)
                + "\nDelay : " + CString(m_delay)
                + "\nEach : " + CString(m_each)
                + "\nMessage : " + m_sMessage
                + "\nCurrent : " + CString(m_current_value)
                + "\nPrevious : " + CString(m_previous_value)
                + "\nMinimum : " + CString(m_minimum_value)
                + "\nMaximum : " + CString(m_maximum_value)
                + "\nLast change : " + GetLastChangeTime(user));
    }

    CTable GetInfosTable(CUser* user) {
        CTable tableInfos = CTable();
        tableInfos.AddColumn("Attribute");
        tableInfos.AddColumn("Value");
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Name");
        tableInfos.SetCell("Value", m_sName);
        tableInfos.AddRow();
        tableInfos.SetCell("Attribute", "Created at");
        tableInfos.SetCell("Value", GetCreationTime(user));
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
        tableInfos.SetCell("Attribute", "Each");
        tableInfos.SetCell("Value", CString(m_each));
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
        tableInfos.SetCell("Value", GetLastChangeTime(user));
        return tableInfos;
    }

    CString GetName() {
        return m_sName;
    }

    int GetDelay() {
        return m_delay;
    }

    CString GetCreationTime(CUser* user) {
        return CUtils::FormatTime(m_creation_datetime, FORMAT_TIME, user->GetTimezone());
    }

    CString GetLastChangeTime(CUser* user) {
        return CUtils::FormatTime(m_last_change, FORMAT_TIME, user->GetTimezone());
    }

    bool HasActiveCooldown() {
        return m_time_chrono < m_cooldown && m_cooldown != 0;
    }

    bool HasMinimumInterval() {
        int diff = abs(m_current_value - m_previous_printed_value);
        return diff >= m_each;
    }

    int GetCurrentValue() {
        return m_current_value;
    }

    int GetPreviousValue() {
        return m_previous_value;
    }

    int GetMinimumValue() {
        return m_previous_value;
    }

    int GetMaximumValue() {
        return m_maximum_value;
    }

    double GetDiffTime() {
        return m_time_chrono;
    }

    CString GetNamedFormat(CString sMessage, CString prefix) {
        MyMap::GetInstance()["NAME"] = m_sName;
        MyMap::GetInstance()["INITIAL"] = CString(m_initial);
        MyMap::GetInstance()["STEP"] = CString(m_step);
        MyMap::GetInstance()["COOLDOWN"] = CString(m_cooldown);
        MyMap::GetInstance()["DELAY"] = CString(m_delay);
        MyMap::GetInstance()["EACH"] = CString(m_each);
        MyMap::GetInstance()["PREVIOUS_VALUE"] = CString(m_previous_value);
        MyMap::GetInstance()["CURRENT_VALUE"] = CString(m_current_value);
        MyMap::GetInstance()["MINIMUM_VALUE"] = CString(m_minimum_value);
        MyMap::GetInstance()["MAXIMUM_VALUE"] = CString(m_maximum_value);
        CString namedFormat;
        if (prefix.empty()) {
            namedFormat = CString::NamedFormat(sMessage, MyMap::GetInstance());
        }
        else {
            namedFormat = CString::NamedFormat(sMessage.Replace_n(prefix, ""), MyMap::GetInstance());
        }
        return namedFormat;
    }

    CString GetNamedFormat() {
        return CCounter::GetNamedFormat(m_sMessage, "");
    }


    //SETTERS

    void SetName(const CString sName) {
        m_sName = sName;
    }

    void SetInitial(const int initial) {
        m_initial = initial;
    }

    void SetStep(const int step) {
        m_step = step;
    }

    void SetCooldown(const int cooldown) {
        m_cooldown = cooldown;
    }

    void SetDelay(const int delay) {
        m_delay = delay;
    }

    void SetEach(const int each) {
        m_each = each;
    }

    void SetMessage(const CString& sMessage) {
        m_sMessage = sMessage;
    }

    /**
     * Reset the counter at resetValue.
     * @param resetValue the value that counter will take.
     */
    void Reset(const int resetValue) {
        PreChangeValue();
        m_current_value = resetValue;
        ResetValues();
    }

    void ResetDefault() {
        Reset(m_initial);
    }

    void Recreate(const int recreateValue) {
        PreChangeValue();
        m_current_value = recreateValue;
        ResetValues();
        SetCurrentValuePrinted();
    }

    void RecreateDefault() {
        Recreate(m_initial);
    }

    void Increment(int step) {
        PreChangeValue();
        m_current_value += step;
        PostChangeValue();
    }

    void IncrementDefault() {
        Increment(m_step);
    }

    void Decrement(const int step) {
        PreChangeValue();
        m_current_value -= step;
        PostChangeValue();
    }

    void DecrementDefault() {
        Decrement(m_step);
    }

    void SetCurrentValuePrinted() {
        m_previous_printed_value = m_current_value;
    }

};

class CImbrication {
protected:
    CCounter *mp_mainCounter;
    CCounter *mp_subCounter;
    CString m_sMessage;

    int m_startValue;
    int m_endValue;

public:

    CImbrication(CCounter *pMainCounter, CCounter *pSubCounter, CString sMessage) :
    mp_mainCounter(pMainCounter), mp_subCounter(pSubCounter), m_sMessage(sMessage) {
        m_endValue = m_startValue = 0;
    }

    void StartSession() {
        m_startValue = mp_subCounter->GetCurrentValue();
    }

    void StopSession() {
        m_endValue = mp_subCounter->GetCurrentValue();
    }

    CString GetNamedFormat() {
        MyMap::GetInstance()["DIFF"] = CString(m_endValue - m_startValue);
        MyMap::GetInstance()["START"] = CString(m_startValue);
        MyMap::GetInstance()["END"] = CString(m_endValue);
        MyMap::GetInstance()["{SUB"] = "{{SUB}";
        CString sNamedFormat = mp_mainCounter->GetNamedFormat(m_sMessage, "{MAIN}");
        sNamedFormat = mp_subCounter->GetNamedFormat(sNamedFormat, "{SUB}");
        //reset map to avoid using diff, start or end variable in simple counter
        MyMap::GetInstance().clear();
        return sNamedFormat;
    }

    int GetStartValue() {
        return m_startValue;
    }

    int GetEndValue() {
        return m_endValue;
    }

    CCounter* GetMainCounter() {
        return mp_mainCounter;
    }

    CCounter* GetSubCounter() {
        return mp_subCounter;
    }
};

#ifdef HAVE_PTHREAD

class CCounterJob : public CModuleJob {
protected:

    CCounter *mp_counter;
    CString m_sAdditionalMessage;


public:

    CCounterJob(CModule* pModule, CCounter *counter, CString sAdditionalMessage) : CModuleJob(pModule, "counters",
    "Send message for counter on channel after a delay"), mp_counter(counter), m_sAdditionalMessage(sAdditionalMessage) {

    }

    CCounterJob(CModule* pModule, CCounter *counter) : CCounterJob(pModule, counter, "") {

    }

    virtual ~CCounterJob() override {

    }

    virtual void runThread() override {
        int delay = mp_counter->GetDelay();
        for (int i = 0; i < delay; i++) {
            if (wasCancelled()) {
                return;
            }
            sleep(1);
        }
    }

    virtual void runMain() override {
        CString formattedMessage = mp_counter->GetNamedFormat();
        CIRCNetwork* network = GetModule()->GetNetwork();
        std::vector<CChan*> channels = network->GetChans();
        for (CChan* channel : channels) {
            GetModule()->PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage + " " + m_sAdditionalMessage);
        }
        mp_counter->SetCurrentValuePrinted();
    }

};
#endif

class CCountersMod : public CModule {
protected:
    //DATA MEMBERS
    std::map<CString, CCounter> m_counters;
    /**
     * map with keys as couple (nickname,listener_name) and value as counter_name
     */
    std::map<std::pair<CString, CString>, CString> m_listeners;
    std::map<CString, CImbrication*> m_imbrications;
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
    T ConvertWithDefaultValue(const CString text, const T value) {
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
    CString CheckStringValue(const CString text, CString defaultText) {
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
    void CreateCounter(const CString& sName, const int initial, const int step,
            const int cooldown, const int delay, const int each, const CString& sMessage) {
        if (!m_counters.count(sName)) {
            CCounter addCounter = CCounter(sName, initial, step, cooldown, delay, each, sMessage);
            auto created = m_counters.insert(std::pair<CString, CCounter>(sName, addCounter));
            if (created.second) {
                PutModule("Counter '" + addCounter.GetName() + "' created.");
                PutUser("/counters counter " + addCounter.GetName() + " " + CString(addCounter.GetCurrentValue()));
            }
        }
        else {
            PutModule("Counter '" + sName + "' already exists.");
        }
    }

    void CreateListener(const CString sName, const CString sNickname, const CString sListenerName) {
        auto created = m_listeners.insert(std::make_pair(std::make_pair(sNickname, sListenerName), sName));
        if (created.second) {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname +
                    "' and counter '" + sName + "' created.");
            PutUser("/counters listener " + sListenerName + " " + sName + " " + sNickname);
        }
    }

    void DeleteListener(const CString sNickname, const CString sListenerName) {
        std::map<std::pair<CString, CString>, CString>::size_type removed = m_listeners.erase(std::make_pair(sNickname, sListenerName));
        if (removed) {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname + "' deleted.");
            PutUser("/counters listener del " + sListenerName + " " + sNickname);
        }
        else {
            PutModule("Listener '" + sListenerName + "' for user '" + sNickname + "' not found.");
        }
    }

    void CreateImbrication(const CString& sImbricationName, CCounter* pMainCounter, CCounter* pSubCounter, CString& sImbricationMessage) {
        CImbrication *imbrication = new CImbrication(pMainCounter, pSubCounter, sImbricationMessage);
        auto created = m_imbrications.insert(std::pair<CString, CImbrication*>(sImbricationName, imbrication));
        if (created.second) {
            CString sSubCounterName = pSubCounter->GetName();
            CString sMainCounterName = pMainCounter->GetName();
            PutModule("Imbrication of counter " + sSubCounterName + " for counter "
                    + sMainCounterName + " with imbrication " + sImbricationName + " created.");
            PutUser("/counters imbrication " + sImbricationName + " " + sMainCounterName + " " + sSubCounterName);
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
                    OnModCommand(sCommand + " " + counter.GetName() + " " + sArgs);
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
     * TODO : maybe parse sCommand with OptionSplit instead of argparse
     * @param sCommand command written by user to parse
     */
    void CreateCounterCommand(const CString& sCommand) {
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
        //retrieve all arguments as strings because i get std::bad_cast with other typenames like int
        CString sInitial = CString(m_parserCreate.retrieve<std::string>("initial"));
        CString sStepValue = CString(m_parserCreate.retrieve<std::string>("step"));
        CString sCooldownValue = CString(m_parserCreate.retrieve<std::string>("cooldown"));
        CString sDelayValue = CString(m_parserCreate.retrieve<std::string>("delay"));
        CString sEachValue = CString(m_parserCreate.retrieve<std::string>("each"));
        CString sMessage = CString(m_parserCreate.retrieve<std::string>("message"));
        CString sName = CString(m_parserCreate.retrieve<std::string>("name"));

        CreateCounter(CheckStringValue(sName, "counter"), ConvertWithDefaultValue(sInitial, DEFAULT_INITIAL),
                ConvertWithDefaultValue(sStepValue, DEFAULT_STEP), ConvertWithDefaultValue(sCooldownValue, DEFAULT_COOLDOWN),
                ConvertWithDefaultValue(sDelayValue, DEFAULT_DELAY), ConvertWithDefaultValue(sEachValue, DEFAULT_EACH),
                CheckStringValue(sMessage, DEFAULT_MESSAGE));
        m_parserCreate.clearVariables();
    }

    void DeleteCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        std::map<CString, CCounter>::size_type erased = m_counters.erase(sName);
        if (erased) {
            PutModule("Counter '" + sName + "' deleted.");
            PutUser("/counters counter del " + sName);
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
    void ExecuteSimpleCommand(const CString& sCommand, std::function<void(CCounter&, int) > execute,
            std::function<void(CCounter&) > executeWithDefault) {
        CString sName = sCommand.Token(1);
        CString sStep;
        if (!sCommand.Token(2).StartsWith("-m")) {
            sStep = sCommand.Token(2);
        }
        CString sMessage = "";
        MCString msOptions = MCString();
        sCommand.OptionSplit(msOptions);
        std::map<CString, CString>::iterator itMessage = msOptions.find("-m");
        if (itMessage != msOptions.end()) {
            sMessage = itMessage->second;
        }
        if (!sName.empty()) {
            try {
                CCounter& counter = m_counters.at(sName);
                if (sStep.empty()) {
                    executeWithDefault(counter);
                }
                else {
                    execute(counter, sStep.ToInt());
                }
                if (!counter.HasActiveCooldown() && counter.HasMinimumInterval()) {
#ifdef HAVE_PTHREAD
                    AddJob(new CCounterJob(this, &counter, sMessage));
#else
                    CString formattedMessage = counter.getNamedFormat();
                    PutModule(formattedMessage);
                    CIRCNetwork *network = GetNetwork();
                    std::vector<CChan*> channels = network->GetChans();
                    for (CChan* channel : channels) {
                        PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage + " " + sMessage);
                    }
                    counter.SetCurrentValuePrinted();
#endif
                }
                PutUser("/counters counter " + counter.GetName() + " " + CString(counter.GetCurrentValue()));
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter " + sName + " not found.");
            }
        }
    }

    void ResetCounterCommand(const CString& sCommand) {
        ExecuteSimpleCommand(sCommand, &CCounter::Reset, &CCounter::ResetDefault);
    }

    void RecreateCounterCommand(const CString& sCommand) {
        ExecuteSimpleCommand(sCommand, &CCounter::Recreate, &CCounter::RecreateDefault);
    }

    void IncrementCounterCommand(const CString& sCommand) {
        ExecuteSimpleCommand(sCommand, &CCounter::Increment, &CCounter::IncrementDefault);
    }

    void DecrementCounterCommand(const CString& sCommand) {
        ExecuteSimpleCommand(sCommand, &CCounter::Decrement, &CCounter::DecrementDefault);
    }

    void PrintCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = m_counters.at(sName);
            CString formattedMessage = counter.GetNamedFormat();
            CIRCNetwork* network = GetNetwork();
            std::vector<CChan*> channels = network->GetChans();
            for (CChan* channel : channels) {
                PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
            }
            PutUser("/counters counter " + sName + " " + CString(counter.GetCurrentValue()));
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter '" + sName + "' not found.");
        }
    }

    void InfoCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        try {
            CCounter& counter = m_counters.at(sName);
            PutModule(counter.GetInfosTable(GetUser()));
        }
        catch (const std::out_of_range oor) {
            PutModule("Counter " + sName + " not found.");
        }
    }

    //TODO : to improve or change because it's possible to change counter's name
    //but this doesn't change used name in the map m_counters

    void SetPropertyCounterCommand(const CString& sCommand) {
        VCString vsArgs;
        sCommand.Split(" ", vsArgs, false, "\"", "\"", true, true);
        try {
            CString sName = vsArgs.at(1);
            CString sProperty = vsArgs.at(2);
            CString sValue = vsArgs.at(3);
            try {
                CCounter& counter = m_counters.at(sName);
                if (sProperty.Equals("NAME"))
                    counter.SetName(sValue);
                else if (sProperty.Equals("INITIAL"))
                    counter.SetInitial(ConvertWithDefaultValue(sValue, DEFAULT_INITIAL));
                else if (sProperty.Equals("STEP"))
                    counter.SetStep(ConvertWithDefaultValue(sValue, DEFAULT_STEP));
                else if (sProperty.Equals("COOLDOWN"))
                    counter.SetCooldown(ConvertWithDefaultValue(sValue, DEFAULT_COOLDOWN));
                else if (sProperty.Equals("DELAY"))
                    counter.SetDelay(ConvertWithDefaultValue(sValue, DEFAULT_DELAY));
                else if (sProperty.Equals("EACH"))
                    counter.SetEach(ConvertWithDefaultValue(sValue, DEFAULT_EACH));
                else if (sProperty.Equals("MESSAGE"))
                    counter.SetMessage(sValue);
                else
                    PutModule("Incorrect property ! Possibles properties are : name, "
                        "initial, step, cooldown, delay, each and message.");

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

    void ListCountersCommand(const CString& sCommand) {
        CString sCounters = "Your counters : ";
        std::map<CString, CCounter>::iterator it;
        for (it = m_counters.begin(); it != m_counters.end(); it++) {
            sCounters.append(it->first);
            if (it != std::prev(m_counters.end())) {
                sCounters.append(", ");
            }
            PutUser("/counters counter " + it->first + " " + CString(it->second.GetCurrentValue()));
        }
        PutModule(sCounters);
    }

    //LISTENERS COMMANDS

    void CreateListenerCommand(const CString& sCommand) {
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
            CreateListener(sName, sNickname, sListenerName);
        }
        else {
            PutModule("Counter '" + sName + "' not found.");
        }
    }

    void DeleteListenerCommand(const CString& sCommand) {
        CString sNickname = sCommand.Token(1);
        CString sListenerName = sCommand.Token(2);
        DeleteListener(sNickname, sListenerName);
    }

    void ListListenersCommand(const CString& sCommand) {
        CString sListeners = "Your listeners : ";
        for (std::map<std::pair<CString, CString>, CString>::const_iterator it = m_listeners.cbegin(); it != m_listeners.cend(); ++it) {
            sListeners.append(it->first.second + " for user " + it->first.first + " and counter " + it->second);
            if (it != std::prev(m_listeners.cend())) {
                sListeners.append(", ");
            }
            PutUser("/counters listener " + it->first.second + " " + it->second + " " + it->first.first);
        }
        PutModule(sListeners);
    }

    //IMBRICATION NESTED-COUNTERS COMMANDS

    void ImbricateCountersCommand(const CString& sCommand) {
        CString sMainCounterName = sCommand.Token(1);
        CString sSubCounterName = sCommand.Token(2);
        CString sImbricationName = sCommand.Token(3);
        CString sImbricationMessage = sCommand.Token(4, true);
        if (!m_imbrications.count(sImbricationName)) {
            std::map<CString, CCounter>::iterator itMain = m_counters.find(sMainCounterName);
            std::map<CString, CCounter>::iterator itSub = m_counters.find(sSubCounterName);
            bool bMainCounterExists = itMain != m_counters.end();
            bool bSubCounterExists = itSub != m_counters.end();
            if (!bMainCounterExists) {
                PutModule("Counter " + sMainCounterName + " not found.");
            }
            if (!bSubCounterExists) {
                PutModule("Counter " + sSubCounterName + " not found.");
            }
            if (bMainCounterExists && bSubCounterExists && !sImbricationName.empty()) {
                if (sImbricationMessage.empty()) {
                    sImbricationMessage = DEFAULT_IMBRICATION_MESSAGE;
                }
                CreateImbrication(sImbricationName, &(itMain->second), &(itSub->second), sImbricationMessage);
            }
        }
        else {
            PutModule("Imbrication " + sImbricationName + " already exists.");
        }
    }

    void StartSessionCommand(const CString& sCommand) {
        CString sImbricationName = sCommand.Token(1);
        try {
            CImbrication *imbrication = m_imbrications.at(sImbricationName);
            imbrication->StartSession();
        }
        catch (const std::out_of_range oor) {
            PutModule("Imbrication " + sImbricationName + "not found.");
        }
    }

    void StopSessionCommand(const CString& sCommand) {
        CString sImbricationName = sCommand.Token(1);
        try {
            CImbrication *imbrication = m_imbrications.at(sImbricationName);
            imbrication->StopSession();
            //le compteur sub doit avoir un délai pour pouvoir envoyer les deux messages sur Twitch
            IncrementCounterCommand("incr " + imbrication->GetMainCounter()->GetName());
            CString formattedMessage = imbrication->GetNamedFormat();
            PutModule(formattedMessage);
            CIRCNetwork* network = GetNetwork();
            std::vector<CChan*> channels = network->GetChans();
            for (CChan* channel : channels) {
                PutIRC("PRIVMSG " + channel->GetName() + " :" + formattedMessage);
            }
        }
        catch (const std::out_of_range oor) {
            PutModule("Imbrication " + sImbricationName + "not found.");
        }
    }

    void DeleteImbricationCommand(const CString& sCommand) {
        CString sImbricationName = sCommand.Token(1);
        std::map<CString, CImbrication*>::iterator it = m_imbrications.find(sImbricationName);
        if (it != m_imbrications.end()) {
            delete it->second;
            PutUser("/counters imbrication del " + sImbricationName);
            m_imbrications.erase(it);
        }
    }

    void ListImbricationsCommand(const CString& sCommand) {
        CString sImbrications = "Your nested-counters : ";
        std::map<CString, CImbrication*>::iterator it;
        for (it = m_imbrications.begin(); it != m_imbrications.end(); it++) {
            CImbrication *pImbrication = it->second;
            CCounter *pMain = pImbrication->GetMainCounter();
            CCounter *pSub = pImbrication->GetSubCounter();
            sImbrications.append("imbrication " + it->first + " with counters "
                    + pMain->GetName() + "(main) and " + pSub->GetName() + "(sub)\n");
            PutUser("/counters imbrication " + it->first + " " + pMain->GetName() + " " + pSub->GetName());
        }
        PutModule(sImbrications);
    }

    //GENERAL COMMANDS

    void ListAllCommand(const CString& sCommand) {
        ListCountersCommand(sCommand);
        ListListenersCommand(sCommand);
        ListImbricationsCommand(sCommand);
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
        m_parserCreate.addArgument("-e", "--each", 1, true);
        m_parserCreate.addArgument("-m", "--message", 1, true);
        m_parserCreate.addFinalArgument("name", 1, false);

        AddHelpCommand();
        //COMMAND FOR COUNTERS
        AddCommand("Create", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::CreateCounterCommand),
                "[--initial | -i <initial>] [--step | -s <step>] [--cooldown | -c <cooldown>]"
                "[--delay | -d <delay>] [--message | -m \"<message>\"] <name>",
                "Create a counter.");
        AddCommand("Delete", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::DeleteCounterCommand),
                "<name>", "Delete <name> counter.");
        AddCommand("Recreate", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::RecreateCounterCommand),
                "<name>", "Recreate <name> counter (reset all values).");
        AddCommand("Reset", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ResetCounterCommand),
                "<name> [reset_value]", "Reset <name> counter.");
        AddCommand("Incr", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::IncrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
        AddCommand("Decr", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::DecrementCounterCommand),
                "<name> [step]", "Decrement <name> counter.");
        AddCommand("Info", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::InfoCounterCommand),
                "<name>", "Show information of <name> counter.");
        AddCommand("Set", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::SetPropertyCounterCommand),
                "<name> <property> <value>", "Set property <property> to <value> for counter <name>.");
        AddCommand("Print", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::PrintCounterCommand),
                "<name>", "Print message for <name> counter.");
        AddCommand("List", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ListCountersCommand),
                "", "List counters.");
        //COMMANDS FOR LISTENERS
        AddCommand("CreateListener", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::CreateListenerCommand),
                "<name> <nickname> <listener_name>", "Create a listener : alias that can be used "
                "on any IRC client (like Twitch).");
        AddCommand("DeleteListener", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::DeleteListenerCommand),
                "<nickname> <listener_name>", "Delete a listener.");
        AddCommand("ListListeners", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ListListenersCommand),
                "", "List listeners.");
        //NESTED-COUNTERS / IMBRICATIONS
        AddCommand("Imbricate", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ImbricateCountersCommand),
                "<main_counter> <sub_counter> <imbrication_name> <message>", "Imbricate counters.");
        AddCommand("DeleteImbrication", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::DeleteImbricationCommand),
                "<imbrication>", "Delete imbrication but not counters.");
        AddCommand("Start", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::StartSessionCommand),
                "<imbrication>", "Start session.");
        AddCommand("Stop", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::StopSessionCommand),
                "<imbrication>", "Stop session.");
        AddCommand("ListImbrications", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ListImbricationsCommand),
                "", "List imbrications.");
        //GENERAL COMMANDS
        AddCommand("ListAll", static_cast<CModCommand::ModCmdFunc> (&CCountersMod::ListAllCommand),
                "", "List counters with their value.");
    }

    virtual bool OnLoad(const CString& sArgs, CString& sMessage) override {
        return true;
    }

    virtual ~CCountersMod() {
        std::map<CString, CImbrication*>::iterator it;
        for (it = m_imbrications.begin(); it != m_imbrications.end(); it++) {
            delete it->second;
        }
        m_imbrications.clear();
    }

};

NETWORKMODULEDEFS(CCountersMod, "Module to count things using commands")
