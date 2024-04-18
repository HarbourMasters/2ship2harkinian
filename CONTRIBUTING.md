# Contributing

### Guiding Principles
strive to preserve the original game code (decomp), such as code paths

present a pleasant out-of-the-box experience for MM with modern quality life improvements/optional enhancements

blah blah then point to how-to or modding.md

### Codebase organization

- root
  - `libultraship/` Submodule
  - `ZAPDTR/` Submodule
  - `OTRExporter/` Submodule
  - `mm/`
    - `src/` Original decomp codebase
    - `2s2h/` Our stuff
      - `*.cpp/h` High level porting code/externed headers
      - `BenGui/` UI/ImGui related code
      - `DeveloperTools/` Developer QoL and debugging tools, not usually the types of things used in a playthrough of the game
      - `Enhancements/` Code related to enhancing the players experience
      - `Randomizer/` copium.jpg

### Where does _your_ code go

not in /src/

when to use c vs c++, externing

header importing

touching source should be a last resort, and should leave minimal footprint with extensive commenting

Your order of operations when attempting to make a new enhancement/feature should be. The further down this list you get the more into danger territory you are
- Make changes entirely isolated from the source, just utilizing hooks that currently exist.
- Introduce new hooks where you might need them, opting to make them as generic as possible so that other developers might take advantage of them.
- Directly call into your isolated code from source, eg `DrawCollisionViewer()`
- Make the changes in the source, wrapping them in a CVar ensuring the vanilla behavior is the default (THIS SHOULD BE A LAST RESORT, AND WILL LIKELY PREVENT YOUR CHANGE FROM BEING MERGED)

### Submodules

* making LUS changes
  * pure lus change
    * pr to LUS
    * testing PR to 2ship with submodule update (draft or marked as DO NOT MERGE)
    * link the prs to each other
    * once LUS pr lands, either update testing PR to use upstream LUS or just wait for an LUS bump
  * changes in both
    * pr to LUS
    * pr to 2ship with submodule from fork (draft or marked as DO NOT MERGE)
    * link PRs to each other
    * once LUS pr lands update 2ship PR to use upstream submodule (undraft/remove DO NOT MERGE)
* making OTRExporter/ZAPDTR changes
    * OTRExporter and ZAPDTR are shared between soh and 2ship so changes need to be verified working against both projects
        * pr to OTRExporter/ZAPDTR
        * test changes for both soh and 2ship (unless the change is only)
        * pr to 2ship and link with the other PR(s) (draft/mark as DO NOT MERGE)
        * once OTRExporter/ZAPDTR PRs lands, update 2ship PR to use upstream submodule commit (undraft/remove DO NOT MERGE)

### Naming

function/variable names, cvar names, file names

prefix functions/classes with a name that describe the system (e.g. SaveManager_XYZ)

For externed variables, try to be more verbose to avoid namespace collisions

cvars should all be sectioned into their associated places, and names should prefer verbosity and clarity over short and/or funny

### Commenting
Don't comment the what, comment the why, etc
Not everything in cpp/2ship land needs to be thoroughly documented but everything in `src/` does

`// 2S2H [Tag] My Comment here`
explain tags, and when to use regions and when just a single comment is fine

port=something we had to do to get the port working, eg change the type of a u32 to uintptr_t
enhancement=just enables an enhancement, eg dpad items
cosmetic=purely cosmetic changes, eg cosmetic editor
etc

### Branches/Releases

What `develop` is, what `develop-<named release>` is, what our release process looks like

what counts as a breaking change

what determines major vs minor vs patch

### Landing your changes

Review process

we squash on merge

committing, PR descripitons, timeline expectations

Expectations of what will and wont get merged (too big, too "mod" like etc)
    - How much does this touch the source code?
    - What is the cost of maintaining this long term?
    - Does the conflict with/complicate other enhancements?

### Misc topics

#### Pulling in decomp changes or documentation
we want this, and we want it to be as painless as possible

#### Porting to other platforms

What we support and what is allowed upstream vs better off as a standalone fork

PORTING.md

#### 