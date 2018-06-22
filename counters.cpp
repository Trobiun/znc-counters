#include <znc/znc.h>
#include <znc/IRCNetwork.h>
#include <znc/Chan.h>
#include <vector>
#include <string>
#include <ctime>
#include "argparse.hpp"


class CCounter {
protected:
    //constants defined by constructor
    CString m_sName;
    int m_initial;
    int m_step;
    int m_cooldown;
    int m_delay;
    CString m_sMessage;
    
    //values that can change
    int m_current_value;
    int m_previous_value;
    int m_minimum_value;
    int m_maximum_value;
    std::time_t m_last_change;
    
    
    std::time_t m_creation_datetime;
    
    
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
        return CString("Nom : " + m_sName + "\nInitial : " + CString(m_initial) + "\nStep : " + CString(m_step)
                + "\nCooldown : " + CString(m_cooldown) + "\nDelay : " + CString(m_delay) + "\nMessage : " + m_sMessage);
    }
    
    CString getCounterName() {
        return this->m_sName;
    }
    
    CString getCreationTime() {
        char mbstr[20];
        if (std::strftime(mbstr, sizeof(mbstr), "%Y/%m/%d %H:%M:%S", std::localtime(&this->m_creation_datetime))) {
            return CString(mbstr);
        }
        return "Unknown date";
    }
    
    int getCurrentValue() {
        return this->m_current_value;
    }
    
    
    //SETTERS
    void preChangeValue() {
        this->m_previous_value = this->m_current_value;
    }
    
    void increment(const int step) {
        this->preChangeValue();
        this->m_current_value += step;
    }
    
    void increment() {
        this->increment(this->m_step);
    }
    
    void decrement(const int step) {
        this->preChangeValue();
        this->m_current_value -= step;
    }
    
    void decrement() {
        this->decrement(this->m_step);
    }
    
};

class CCountersMod : public CModule {
protected:
    //ATTRIBUTES
    std::map<CString,CCounter> m_counters;
    ArgumentParser m_parserCreate;
    
    template<typename T>
    T convertWithDefaultValue(const CString& text, const T value) {
        std::stringstream ss(text);
        T result;
        return ss >> result ? result : value;
    }
    
    CString checkStringValue(const CString text, CString defaultText) {
        return text.size() > 0 ? text : defaultText;
    }
    
    void createCounter(const CString& sName, const int initValue = 0, const int stepValue = 1,
    const int cooldownValue = 0, const int delayValue = 0, const CString& sMessage = "[NAME] has value : [CURRENT_VALUE]") {
        CCounter addCounter = CCounter(sName, initValue, stepValue, cooldownValue, delayValue, sMessage);
        this->m_counters.insert(std::pair<CString,CCounter>(sName, addCounter));
        PutModule("Compteur ajouté : " + addCounter.getCounterName());
        PutModule("Compteur créé le : " + addCounter.getCreationTime());
        PutModule(addCounter.prettyPrint());
    }
    
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
        
        
        VCString vsArgs3;
        CString::size_type tokensNb3 = sCommand.Split(" ", vsArgs3, true, "\"", "\"", true, true);
        PutModule("Commande séparée en : " + CString(tokensNb3) + " chaines avec Split.");
        std::vector<std::string> args3 = std::vector<std::string>();
//        args3.push_back(sCommand.Token(0));
//        std::vector<const char*> argv;
//        argv.reserve(tokensNb3 + 1);
        for (const CString& arg : vsArgs3) {
//            std::string *tempString = new std::string();
//            bool convertSuccess = arg.Convert<std::string>(tempString);
//            if (convertSuccess) {
//                PutModule("Conversion réussie de " + arg + " à " + *tempString);
//                args3.push_back(tempString->c_str());
//                argv.push_back(const_cast<char*>(tempString->data()));
//            }
            if (arg.size() > 0) {
                PutModule("Argument : " + arg);
                args3.push_back((std::string)arg);
            }
            else {
                PutModule("Argument vide");
//                args3.push_back("NULL");
            }
            
//            delete tempString;
        }
//        args3.push_back(nullptr);
//        argv.push_back(NULL);
//        PutModule("Fin.");
        try {
            this->m_parserCreate.parse(args3);
        }
        catch (std::invalid_argument ex) {
            PutModule("error invalid argument : " + CString(ex.what()));
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
    
    void incrementCounterCommand(const CString& sCommand) {
        CString sName = sCommand.Token(1);
        CString sStep = sCommand.Token(2);
        if (sName.size() > 0) {
            try {
                CCounter& counter = this->m_counters.at(sName);
                PutModule("Ancienne valeur : " + CString(counter.getCurrentValue()));
                if (sStep.size() > 0) {
                    PutModule("Incrémentation de : " + sStep);
                    counter.increment(sStep.ToInt());
                }
                else {
                    PutModule("Incrémentation par défaut.");
                    counter.increment();
                }
                PutModule("Valeur courante : " + CString(counter.getCurrentValue()));
            }
            catch (const std::out_of_range oor) {
                PutModule("Counter not found.");
            }
        }
//        PutModule("Nom : " + sName);
//        PutModule("Step : " + sStep);
//        CCounter counter = this->m_counters.at(sName);
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
        AddCommand("incr", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::incrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
        AddCommand("+", static_cast<CModCommand::ModCmdFunc>(&CCountersMod::incrementCounterCommand),
                "<name> [step]", "Increment <name> counter.");
    }

    virtual bool OnLoad(const CString& sArgs, CString& sMessage) override {
        return true;
    }

    virtual ~CCountersMod() {
        
    }

};

NETWORKMODULEDEFS(CCountersMod, "Module to count things using commands")