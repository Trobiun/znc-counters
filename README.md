# ZNC Counters
A ZNC (1.6.x version) module to count things using commands.

## Counters
Obviously this is the main object used in this module. It can increment, decrement and reset
### Commands
- `create [(--initial | -i) <initial>] [(--step | -s) <step>] [(--cooldown | -c) <cooldown>] [(--delay | -d) <delay>] [(--message | -m) "<messsage>"] <name>`

  Create a counter.
- `delete <name>`

  Delete a counter if it exists.
- `reset <name> [<value>]`

  Reset a counter, with value if specified otherwise with initial value of counter. Variables like previous, current, minimum and maximum values will be reseted to the reset value.
- `incr <name> [<step>]`

  Increment a counter by step if specified, by step value of counter otherwise.
- `decr <name> [<step>]`

  Decrement a counter by step if specified, by step value of counter otherwise.
- `set <name> <property> <value>`

  Set a property of counter. (possible values as property are : initial, step, cooldown, delay and message)
- `info <name>`

  Show information of a counter like its properties and other values like current, previous, minimum and maximul values.
- `print <name>`

  Send the message of a counter without delay.
- `list`

  List all existing counters.

## Listeners
It consists to use counters with a sort of alias, but it can be used by others users who are not connected to znc server.
### Commands
- `createListener <name> [<nickname>] [<listener_name>]`

  Create a "listener", a sort of alias for a counter that can be used by <nickname> with <listener_name>.
- `deleteListener <nickname> <listener_name>`

  Delete a listener if it exists.
- `listListeners`

  List all existing listeners.

## Variables and default values
Variables that can't be changed manually by user :
- name : the name of the counter, used to identify it
- previous_value : the value before change
- current_value : the current value
- minimum_value : the minimum value reached
- maximum_value : the maximum value reached
  
There are some default values to create counters or listeners.
- For counters :
  - Initial : 0
  - Step : 1
  - Cooldown : 0 (seconds)
  - Delay : 0 (seconds)
  - Message : "{NAME} has value : {CURRENT_VALUE}"
- For Listeners :
  - Nickname : current nickname of user that create listener
  - Listener name : "!" + name of the counter

## Format of message
The following keywords (case sensitive) will be replaced in the message sent by the module with the appropriate value :
- `{NAME}` : the name of the counter
- `{INITIAL}` : the initial value of the counter used at creation and reset
- `{STEP}` : the step value used to increment and decrement the counter
- `{COOLDOWN}` : the cooldown between 2 messages from the counter
- `{DELAY}` : the delay between the change of value and the sending message
- `{PREVIOUS_VALUE}` : the previous value to the current one
- `{CURRENT_VALUE}` : the current value of the counter
- `{MINIMUM_VALUE}` : the minimum value reached
- `{MAXIMUM_VALUE}` : the maximum value reached

## Examples
```
/znc *counters create test
/znc *counters incr test 2  //will send "test has value : 2"
/znc *counters set test message "{NAME} changed from {PREVIOUS_VALUE} to {CURRENT_VALUE}"
/znc *counters reset        //will send "test changed from 2 to 0"

/znc *counters createListener test user !testl
//In the above example, if the user with nick "user" sends a message beginning with "!testl" this will perform
//the rest of the message like a command.
//For example : "user" sends "!testl incr", this will execute "incr" command for the counter "test" and
//send "test changed from 0 to 1"
```

This module uses argparse to parse "create" command : https://github.com/hbristow/argparse
