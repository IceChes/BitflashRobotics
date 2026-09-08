# Bitflash Robotics AI + Example Code Usage Policy
Due to the safety-critical nature of combat robotics firmware, the following guidelines are in place for AI usage.

To contribute to this project, this entire document must be read and strictly adhered to.

## Overview

### LLMs shall not be used for:
- Generation of commit messages or other repository-related pieces of text.
- Generation of documentation.
- Generation of code not marked as created by AI.
- Generation of any code copied and pasted directly into the codebase.

### LLMs are permitted to:
- Generate example material.
- Assist the programmer with unfamiliar syntax.
- Review code and suggest changes.

## Restrictions on Types of LLMs

The only LLMs allowed for this project are locally-hosted, open-weights models. Commercial LLMs are banned for any purpose on account of being owned and operated by unimaginably evil companies. Your usage of commercial AI will end up contributing to some finance officer's statistics and eventually used to justify:
- Construction of gas plants
- Construction of data centers
- Destruction of local economies
- Mass surveillance
- Further international economic damage
- Destruction and arson of books
And other crimes.

## AI-Generated Code
Using LLMs for the express purpose of code generation is prohibited. The only instances where using AI-generated code is permissible are:
- The code is suggested as part of a code review.
- The code is part of an AI-generated piece of example material.

Code synthesized by an LLM must be copied by hand into the codebase. Copying and pasting is not permitted, and any copying must only take place after the user fully understands the code that is being written. Preferably, no AI-generated code is used at all. 

Copying code that you do not understand into a Bitflash Robotics project is a violation of basic safety practices and fundamental engineering ethics. 

## Flagging Code
Fully or mostly AI-generated code must be flagged using the exact template below.

"The following code was generated \<wholly/partially\> by \<model\>. I, \<author\>, forfeit all rights to this code."
\<code\>
"End of AI flag."


```cpp
//The following code was generated partially by Gemma 4. I, iceches, forfeit all rights to this code.
struct DeviceRegistry {
  const char *name;
  const byte id;
  DataProcessor process;
};
//End of AI flag.
```