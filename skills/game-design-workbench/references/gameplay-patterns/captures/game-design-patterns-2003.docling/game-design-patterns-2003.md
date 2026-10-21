## COPYRIGHT

Copyright © 2003 by authors, Utrecht University and Digital Games Research Association (DiGRA).

All rights reserved. Except for the quotation of short passages for the purpose of criticism and review, no part of this publication may be reproduced or utilized in any forms or by any means, electronic or mechanical, including photocopying, filming, recording, or by any information storage and retrieval system, without permission in writing from the copyright holders.

## 15.GAME DESIGN PATTERNS

Staffan Björk Sus Lundgren Jussi Holopainen

## ABSTRACT

We present a model to support the design, analysis, and comparison of games through the use of game design patterns,  descriptions  of  reoccurring  interaction  relevant to game play. The model consists of a structural framework to describe the components of games, and patterns of interaction  that  describes  how  components  are  used  by players (or a computer) to affect various aspects of the game play. Focusing on the patterns and identified methods for using them, we describe the development of the model and how we are currently working to enlarge and validate the collection of patterns.

KEYWORDS

Game Design, Patterns, Taxonomies, Game Models

## INTRODUCTION

The interest for developing a field of game research, ludology, has steadily been growing over the last few years. But games vary greatly, not only in content and game play, but also in medium and why they are played, which offers many approaches to the subject. This can be observed by looking at current research, which is done by applying methods and concepts from a wide range of research fields, e.g. sociology, pedagogy, literature studies, media studies, and computer science. In addition, this work is being done with many different research goals in mind such as answering questions regarding player activities, describing narrative  structures,  finding  best  practices  for  game  development  or  fulfilling artistic challenges. Assuming that a unified approach to studying games is optimal, what framework can encompass this diversity?

In this paper we present a general framework for the study of games based on game design patterns. The paper begins with an overview methods currently used in industry and academic, which form our motivation to use game patterns. We describe the components of our framework together with examples and ways of using the framework. The paper concludes with a discussion on the perceived strengths and weaknesses of the approach as well as future work.

## Industry

Digital  games have become a major industry with the most popular games selling over a million copies each and total yearly sales in the range of billions [39].  To  manage  the  big  projects  that  a  major  game  release  requires,  the industry uses a mixture of techniques and concept borrowed  from  software  development,  the  movie industry, and traditional games. Although this works, as seen from the games reaching the market, there is  an  explicit  concern  among  professional  game designers that a developed design discipline for digital  games  is  lacking  (e.g.  [14,  41]),  especially  one developed  to  support  the  multidisciplinary  groups that  in  practice  create  the  games.  Although  the demands of delivering games on deadline leave little time  for  developers  to  pursuit  research,  there  are examples of designers who work on models for game design (c.f. [15, 43, 48]) but these are few compared to the games being designed, and are seen as exceptions to the normal game designer. Beyond the problem of managing game projects, the game industry has been perceived as stagnating; mainly producing sequels, expansions, conversions to other platforms, or brand-based games (c.f. see the commonality of sequels in sales in [21, 22]). Undeniable an economical  sensible  strategy,  at  least  short-term,  the  successes of repackaging have been described as a challenge to the creativeness of designers [42].

Thus, parts of the game design industry is seeking methods  that  can  bring  more  structure  to  game design, in order to expand the design space of games beyond what has previously been commercially successful. Other current issues the industry is seeking answers to include making games that make full use of the context of new platforms (mobile phones and PDAs),  structured  methods  to  discuss  merits  of  a game design, and knowing what patents exists that can influence the possible choices when creating a concept.

## Academia

Not  counting  mathematically-oriented  subjects such as game theory, most work within the field of game  research  has  either  describes  the  historical development of a game genre, often together with a taxonomy,  (c.f.  [3,  26,  38]  or  explored  the  role  of games from a sociological approach [11] or as cultural  phenomenon  [20].  In  contrast,  the  study  of digital  games  have  often  focused  on  games  as  a medium  for  story-telling  and  thus  been  based  on theories and methods from narrative fields such as literature, theatre, film etc. (c.f. [28, 29, 34, 36, 37, and 46]). Recently, there has been a strong interest from applied research in how new computer technology (c.f. [23]) that has been used to explore new interaction forms within games [2, 5, 6, 12, 16, 19, 40, 44, 45] (or sports [35]). These systems have usually created a new context for game play while those who wish to maintain the traditional game setting have embedded computational technology in traditional  components  [18,  33]  or  cards  and  game boards [30, 32].

Thus, there are many examples of scientific and academic  interest  in  games.  However,  the  results have mostly stayed within one research field, probably due to the highly specialized language within all  research  fields,  which  has  limited  the  development of game research.

## The Need for a Common Language for Games

Looking at the work conducted both within academia and industry, one can conclude that there is a need for  a  language to be able to talk about game both while  designing  games  and  while  analyzing  game play.  To  reap  most benefit from such a language it should be usable by the all interested parties to maximize  knowledge  transfer.  This  makes  it  difficult  to ground any game-centric language in one research discipline  or  engineering  practice.  Although  concepts, methods, and theories from numerous fields can, and should, be incorporated into a conceptual game language we believe that the  foundation  for such  a  language  should  be  created  from  studying games as a phenomenon in itself.

## GAME DESIGN PATTERNS

## RELATED WORK Genres

The use of genres such as sport games, first-person shooters, strategy games etc. are the most common way  to  give  product  information  about  computer games.  However,  the  definition  of  genres  strongly depends on the popularity of various games which is not  surprising  as  the  'genre  conceptions  originate mostly from game journalism, not systematic study.' [24]  Looking  at  the  academic  field,  the  game  taxonomies  mentioned  earlier  [26,  38]  (but  also  [15]) can also be seen as genre collections, although the term  genre  is  not  explicitly  used.  However,  when genre  identification  is  based  on  the  interactivity,  a categorization can easily result in 42 different genres [47], something that has been argued to potentially make their usefulness suffer [25].

Due  to  these  problems  of  trying  to  define  genres that are both generic and relevant within a specific subcategory of games types, we do not propose that a redefinition of the concept of genre would provide a  feasible  basis  for  a  common  language  of  game research.  Instead  we  believe  that  finding  components that can be used to describe genres would be beneficial to all types of categorization of games.

## Game mechanics

A natural starting point in trying to identify the components that constitute a genre is to find the common components in the games that are used to exemplify the  genre.  When  studying  various  communities  of gamers  and  game  designers  we  found  that  many used the concept of mechanics or mechanisms.

However, the definition of a game mechanic is general ('Part of a game's rule system that covers one general or specific aspect of the game' [9]) and not useful for academic research. A typical mechanic is 'roll  and  move'  that  simply  states  that  dice  are

182

rolled and that something else is moved related to the outcome of the die roll. The mechanic does not state how something should be moved or why; this is determined  in  the  rules  for  the  particular  game. Computer  game  designers  also  frequently  use  the term mechanics but the term is not strictly defined it is used both in the way it is used for board games and within technical programming contexts [30].

Even  though  lacking  a  rigorous  definition  the  concept of mechanics, i.e. that a game can be regarded as  an  entity  put  together  by  a  number  of  smaller components, seems to be very useful. However, as has  been  argued  [25,  31],  a  structure  to  define mechanics more rigorously and include information about their relationship as well as how to apply them seems necessary.

## Other related models

In addition to genres and game mechanics, a number of alternative approaches have been suggested, primarily  from  professional  game  designers.  Although they have not been widely applied within either the game industry or academia, they are mention here as they have been important influences to our approach.

Writing  to  a  designer  audience,  Church  [13]  introduced the concept of Formal Abstract Design Tools (FADTs) as a way to reach a shared design vocabulary. Although he stresses the importance of formalism  and  abstracting  away  from  specific  instances, the FADTs are one sentence descriptions. For example,  the  FADT  Perceivable  Consequences is defined simply as 'A clear reaction from the game world to the action of the player.'

Barwood  &amp;  Falstein  have  introduced  400  Design Rules project [4]. The aim of the project is to collect proven game design rules and techniques which are stated  as  instructions.  Consisting  of  the  sections Imperative Statement, Domain  of Application, Dominated Rules,  Dominating  Rules,  and  Examples Aliases the rules are aimed at practical game design and are less suitable for analytic studies.

## DEVELOPMENT OF OUR MODEL

## Theoretical foundation

Most academic research to date has studied games using terms and concepts from narrative fields such as literature, theatre and film. The focus on narrativity that this naturally brings risks that the aspect of interaction is lost; something that can be argued is a more defining characteristic to games than narrative structures.  This  emphasis  of  narrativity  may  have resulted  in  the  limited  success  of  academic  results being adapted by other disciplines and by the industry.  To  avoid  this,  we  wished  to  find  a  basis  for  a game language centered on interaction rather than narratology.  With  interaction  we  mean  both  the interaction between players playing a game and the interaction between players and the game.

As we described in the section above, the use of game mechanics seemed to be a promising starting point to describe interaction elements in games. However, to be  able  to  use  such  collections  of  game  mechanics more  effectively,  a  structure  to  describe  how  they influence each other would be required. Design patterns [1, 10, 17] is a method of codifying design knowledge in separate but interrelated parts and have been used to describe game elements related to interaction [27]. Further, game mechanics can easily be converted to design patterns making it a seemingly ideal candidate for our model. However, design patterns are not ideally  suited  as  analytical  tools  due  to  their  initial introduction as a problem-solving tool:

'Each pattern describes a problem which occurs over and over again in our environment, and then describes the core of the solution to that problem, in such a way that you can use this solution a  million  times  over,  without  ever  doing  it  the same way twice.' [1, p x]

So while design patterns seem to be applicable for our use, we argue that not all aspects of design can or should be seen as solving problems, especially in a  creative  activity  such  as  game  design  which requires not only engineering skills but also art and design  competences.  To  support  these  activities  a redefinition  of  the  pattern  templates  would  be required.

## Empirical development

In order to develop a suitable pattern template, individual  game  design  patterns  and  the  overarched structure we proceeded by gathering data through a variety of methods.

## Transforming Game Mechanics

Given this initial conceptual framework, we proceeded  by  examining  game  mechanics  and  converting them to patterns. This included discarding a number of  mechanics,  merging  some  mechanics  into  one pattern and especially identifying more abstract or more detailed patterns.

## Harvesting Patterns by Analyzing Games

The second approach to create an initial pattern collection  was  by  'brute  force'  analysis  of  existing games, concepts and design methods of other fields (such as architecture, software engineering, evolutionary biology, mathematics, and interaction design), and extrapolating  possible  person-to-person  and  personto-environment interactions from the fields of sociology,  social  psychology,  psychology  and  cognitive  science. Our method for harvesting consists of five iterative steps: recognize, analyze, describe, test and evaluate.  The  recognition  phase  creates  a  quick  pattern candidate collection around a certain idea or interac-

## GAME DESIGN PATTERNS

tion area. The next step is that the collection is analyzed by describing how the pattern is used in example games and then trying to remove the pattern from the games and explaining how it would change the game play.  The  pattern  is  then  described  using  the  developed  pattern  template.  The  description  is  tested  by creating a simple prototype game utilizing the pattern and finally the pattern is evaluated using usefulness and sufficiency of the description as criteria. As the work progressed the strict five step method was transformed  to  a  dynamic,  recursive  one  where  pattern fusion, mutation and creation of new candidates was possible at almost every stage. The different phases, however, were still used but not in a strict sequence. The result was over 200 pattern candidates together with unexplored but promising areas of interaction.

## Interviews

In  order  to  collect  information  about  how  game development uses game concepts we interviewed 9 professional game designers that together represented designers of the full spectrum of game mediums. All  used  the  terms  genre,  theme  and  mechanisms casually;  this  was  clearly  concepts  they  were  very familiar with. However, they didn't mention very many mechanics  by  a  specific  name  (perhaps  because there are no standardized names and no collection). The  typical  exceptions  (for  board  and  card  game developers)  were  Bluff,  Tension,  Action  Cards, Storytelling, Trading, Action Points and Cooperation. Some of the designers were themselves interested in creating structured frameworks for games and several  of  them  were  already  aware  of  design  patterns methodologies.

All  though  the  data  has  not  be  fully  analyzed,  the interviews provided feedback that our proposed solution was compatible with the way developers worked as  well  as  providing  many  concepts  that  could  be developed to become patterns.

184

## AN INTERACTION-CENTRIC MODEL FOR GAMES

The development of our model for games and game play  has  been  alternation  between  working  on  a structural framework that describes the components of  the  game  and  the  game  design  patterns  that describe  player  interaction  while  playing.  Although the  two  parts  are  the  results  of  an  intertwined process they can be used independently; the structural framework can be used without the patterns to describe games and the use of design patterns can be based on other structural frameworks. Due to limited space, we do not present a detailed description of  the  structural  framework  and  refer  interested readers to the companion paper to this paper [8].

## Structural Framework

The  structural  framework  was  developed  from  an initial  analysis  of  how  the  terms  used  to  describe games. This  framework  was  expanded  and  refined by examining the relationship between the terms as well  as  try  to  use  the  structural  framework  to describe games and interaction in games.

On  the  highest  level  of  abstraction  the  structural framework consists of game instance, game session and  play  session  which  logically  and  temporally delimits the activity of playing a game from other activities.  To  describe  the  actual  games,  components are used that belong to one of three different categories: bounding, temporal and objective.

The  bounding  category  consisting  of  goals,  rules, and game modes, is the most abstract and include components that are used to describe what activities  are  allowed  or  not  allowed  in  the  game.  The temporal category consists of actions, events, end conditions, evaluation functions and closures, and describes the temporal execution performed during game play. The objective category consists of players,  interfaces,  and  game  elements  (e.g.  tokens, dice, cards, player avatars, NPCs, movable objects, tiles,  backgrounds)  and  represents  components that  are  physical  (or  in  the  case  of  digital  games virtual). Game elements have control/action structures and information structures (including scores, attributes,  etc.)  which  dictate  how  players  can affect the game and what knowledge they have of the game state.

## Game Design Patterns

Unlike most design patterns we have chosen not to define  patterns  as  a  pure  problem-solution  pairs. This is due to two observations. First, defining patterns  from  problems  creates  a  risk  of  viewing  patterns  as  a  method  for  only  removing  unwanted effects of a design. In other words, using patterns as a tool for problem-solving only and not as a tool to support creative design work. Second, many of the patterns we have identified described a characteristic that more or less automatically guaranteed other characteristics in a game, i.e. the problem described in  a  pattern  might  easily  be  solved  by  applying  a more specific subpattern.

## Name

Although not explicitly stating this in the template, we have in the naming process of patterns aimed at short, specific, and idiomatic names. The main purpose for this was not to provide intuitive names, but names that could  provide  mnemonic  support  after the pattern description had been read. In the cases where patterns were adapted from concepts in other research  fields,  we  have  maintained  that  name  to provide a link to that field. We have deliberately not included  aliases  to  minimize  the  number  of  names that  need  to  be  remembered;  we  instead  take  an approach similar to that of a dictionary by provide synonym-analog in the form of references to similar concepts in other models and fields of study.

## Description

The pattern starts with a concise description of the pattern,  often  with  notes  on  in  which  game  it  was identified  and  if  the  pattern  has  been  identified  in previous  models.  Further,  the  description  contains information on how it affects the structural framework (especially if the pattern can be instantiated on different scales in the game) and examples of games in which the pattern is typically found.

## Consequences

Each solution has its own trade-offs and consequences. Solutions can, in turn, cause or amplify other problems. To take a design decision for or against a given solution, its costs and benefits have to be understood and compared  against  those  of  alternatives.  This  section describes the likely or possible consequences of applying the solution suggested by the pattern.

## Using the Pattern

As patterns are general solutions the application of a pattern to any given situation requires a number of design  choices  specific  for  the  current  context. However, the high-level choices can often be divided into categories. This section is used to mention the common  choices  a  designer  is  faced  with  when applying  a  pattern,  often  exemplified  by  specific game elements from published games.

## Relations

Here  the  relations  between  different  game  design patterns are stated. These are basically three forms of  relationship:  patterns  that  are  superior  in  the sense that they describe more abstract characteristics (often mentioned in the consequences section) and can be implemented by applying the given pattern, subpatterns that can be used to implement the given pattern (often mentioned in the using the pattern section), and conflicting patterns that are difficult to implement with the given pattern.

## GAME DESIGN PATTERNS

## Pattern examples

During  our  work  we  have  found  over  200  game design  patterns  which  we  are  currently  describing and testing. To give better understanding to our patterns, we present one  pattern below whose effect on games have been described several times in  other forms (see the references in the description). Italic texts indicate referenced patterns.

## PAPER ROCK SCISSORS

Description: This  pattern  is  based  on  the  children's  game with the same name. It means that players try to outwit each other by guessing what the other ones will do, and by tricking other players to take a wrong guess on one's own action. The original game is very simple; after a count to three both  players  make  one  out  of  three  gestures, depicting rock, paper or scissors. Rock beats scissors,  scissors  beat  paper  and  paper  beats  rock. That there is no winning strategy is the essence of the pattern: players have to somehow figure out what choice is the best at each moment.

This  game  pattern  is  well-known  with  the game design community (sometimes called 'triangularity',  see  Crawford)  and  is  a  mnemonic name for  the  logical  concept  of  non-transitivity (basically, even if A beats B and B beats C, A doesn't beat C).

Examples: Quake (relation between weapons and  monsters),  Drakborgen,  SimWar,  protogame to show non-transitivity (Dynamics for Designers, Will Wright, GDC 2003)

Consequences: Paper-Rock-Scissors  patterns can  either  be  implemented  so  it  choices  have immediate  consequences  (as  in  the  game  that gave the pattern its name) or long-term effects. In  both  cases  it  promotes Tension ,  either  until the  moment when the choices are revealed or until the success of the chosen strategies is evident.  A  paper-rock-scissor  pattern  introduces

186

Randomness unless  players  can  either  gain knowledge  about  the  other  players  current activities  or  keep  record  over  other  players behavior,  as  otherwise  a  player  has  no  way  of foreseeing what tactics is advantageous. If the game  supports  knowledge  collection,  the  correct  use  of  the  strategies  allows  for Game Mastery.

Using the Pattern: Games with immediate consequences  of  choices  related  to  Paper-RockScissor  usually  have  these  kinds  of  choices often  in  the  game  to  allow  people  to  keep records over other player behavior. Quick Games using the pattern, such as the game which lent its  name  to  the  pattern,  usually  are  played repeatedly so some form of Meta Game can be used to allow players to gain knowledge of their opponents' strategies.

A  common  way  to  implement  the  pattern for having long-term effects is through Investments to  gain Asymmetrical  Abilities , either through Proxies or C haracter Development .  See Dynamics for Designers (Will Wright)  for  an  example  based  on  proxies.  For this  kind  of  use  of  the  pattern,  players  can  be given  knowledge  about  other  players  through Public Information or in the case of games with Fog  of  War through  sending Proxies .  Allowing players  to  keep  record  over  other  players' behavior  is  trivial  if  play  commences  face-toface, otherwise some form of Personalization is required.

Relations: Superior patterns are Player Balance,  Tension , Secret Tactics ,  and Game Mastery .  Subpatterns are Trump , Randomness , Asymmetrical Abilities , Public  Information , Investments , Proxies , Character  Development and Meta Game .

## References:

1. Kreimeier, B. The Case For Game Design Patterns. www.gamasutra.com/features/20020313/ kreimeier\_01.htm

2. Wright, W. Dynamics for Designers. Presentation at GDC 2003.

http://www.gdconf.com/archives/2003/ Wright\_Will.ppt

3. Orthogonal Unit Differentiation, Harvey Smith. Presentation  at  GDC  2003.  http://www.gdconf. com/archives/2003/Smith\_Harvey. ppt

4. Chris Crawford.

The Art of Computer Game Design

## APPLYING GAME DESIGN PATTERNS

Unlike  earlier  uses  of  patterns,  we  do  not  propose one single (problem-solving) method for using patterns. Instead, we see the patterns and the structural framework as a tool, similar to a pen, which can be used  in  several  different  ways  for  several  different reasons.  This  is  because  we  see  several  potential user groups which have inherently different working methods. This being said, we have identified a number of different types of uses that patterns can be used to support. Although we have yet to collect substantial amounts of data regarding the feasibility of using various approaches, we do believe that the use areas described below hold potential.

We do not state target users for the various protomethods as we believe that this is highly dependent on  the  specific  use  context  and  how  rigorously  the users structure their use of patterns. For example, the act of categorizing games and genres may seem most suited for academics but could also be used by critics writing  reviews  or  gamers  making  decisions  about purchases. However, we stress that game design patterns are beneficial to multidisciplinary groups as they ease  communication  by  being  neutral  definitions based on the interaction in games and not based on any research field or professional jargon.

## Idea generation

Game developers can use the patterns to give inspiration by simply randomly choosing a set and trying to imagine a game using them. A more structured approach may be to study an individual game design pattern and try to implement it in a novel way.

## Development of game concepts

Once an initial game concept exists, it can be developed  using  patterns.  Describing  the  concept  as  a small set of patterns, it can then be fleshed out and more specific design choices can be made by deciding how to instantiate those patterns through subpatterns and studying how the different design patterns interact. The process can be iteratively refined by examining the chosen subpattern until the preferred level of detail is achieved.

## Pre-production process

Having  a  game  described  using  patterns  offers advantages  when  presenting  the  game  design  to people. Besides allowing a structured description of the design, motivations for particular design choices (describes as patterns) can be done by relating to  other  games  using  the  same  patterns  or  by describing  how  replacing  the  pattern  with  other patterns would change the design. This advantage is increased if the people already have been introduced  to  design  patterns  from  previous  game design as they easier can compare the designs.

## Identifying Competition and IP/patent issues

As a side-benefit of having identified the patterns in a game design, one can identify competition, in the form of what the game will be compared to, by the examples given in the patterns. Further, references in  game design patterns may point to patents that can influence the development of commercial game products.

## GAME DESIGN PATTERNS

## Problem-solving during development

Similar to the rational for FADTs and the 400 rules, game design patterns are a way to collect the knowledge and experience of game developers. As such, they  contain  descriptions  and  motivations  for  how one can modify game designs to solve issues relating to game play in a design.

## Analyzing games

The availability of a pattern collection can provide a simple way to start analyzing an existing game. By simply  iteratively  going  through  the  collection  and see if a pattern exists, or rather, to which degree a pattern exists in a game. Further information about the  game can then be gained by studying if previously identified subpatterns are used to create a pattern or if novel elements have been introduced.

## Categorizing games and genres

Assuming  that  a  patterns-based  analysis  has  been performed on a collection of games, these can then be  categorized  by  their  similarities  or  differences. Besides offering a multitude of dimensions of how to measure in what way games compare to each other, collections of patterns found in games belong to a genre  can  be  used  to  describe  or  understand  that genre.

## Support to explore new platforms and medium

As mentioned in the introduction, the game industry has  due  to  the  economically  successful  model  of sequels and branding become what can paradoxical-

188

ly be called conservative. This lack of going beyond existing frames exists not only in thematic and game play styles but also in platform. We believe that the use of patterns can help the exploration of new types of games and they can provide a structured way to compare  how  game  play  changes  with  a  changed environment. This is especially likely for novel game mediums such as pervasive gaming which is a development of computer games but need to function in social conditions similar to those where more traditional games are played.

## DISCUSSION

Our work with game design patterns is still in its initial stages and as such we have identified several different areas of work required to be able to draw more substantial  conclusion  of  the  feasibility  of  game design patterns in various use areas.

Further, even if a pattern approach satisfies the need for  understanding  games  and  game  design,  some issues may hinder the wide-spread use of patterns in game development and research. In the lack of a collection of suitable patterns, the process of making a pattern  collection  which  would  be  useful  is  difficult and time consuming. Making one large collection containing  all  identified  patterns  in  an  encyclopedic endeavor  may  solve  this  problem  by  containing  all possible  sets  of  required  patterns,  but  finding  the specific patterns in the day to day design work may be  too  time-consuming  especially  as  identified  patterns  may  be  linked  to  many  patterns  that  are  not relevant to a particular case. This problem has led us to start investigating ways of aiding users to quickly identify relevant patterns without an extensive knowhow of the collection, and will probably require different solutions to each of the suggested use areas.

## Validating patterns

To create the pattern collection, we have engaged in various activities as described in the empirical development section. The identification of the same game design  patterns  in  very  different  kinds  of  games (Carcassonne  and  Qix  in  one  example,  Pac-Man  &amp; King of the Hill variants of FPS in another) we believe to  be  indicators  of  the  value  of  patterns  to  understand interaction in games.

The use of patterns in analysis has already proven fruitful  in  analysis  of  the  games  Pac-Man,  Missile Command  &amp;  Mind-sweeper  in  a  research-orient workshop 1 and the patterns have also been used in various experimental game prototypes [7].

However, to validate the analytical, problem-solving and communicative values of patterns they need to be put to use. To support this we are in the process of  making  all  patterns  available  online  as  well  as engaging both industry and academia in workshops focusing game play analysis or experimental game design.

## Creating the pattern collection

One of the problems with creating the design pattern

1 Although not finally analyzed, material from the workshop can be found at http://www.gamedesignpatterns.org.

is determining exactly how much unique information is required for a concept to be a pattern in its own right and not just a variant or comment mentioned in a (superior) pattern. Although we currently flavor an inclusive approach and with an evolutionary refinement  process  based  on  use  and  feedback  from researchers and designers, we note that it might be desirable to have a slightly weak superior pattern if it has several clear and useful subpatterns or to have an insignificant pattern as a separate pattern if it has more than one superior pattern, in order to show the connection.

## Subpatterns &amp; Superior Patterns.

The structure of the pattern collection is not a strict hierarchy  but  a  network  with  several  base  nodes. Although we have not found and circular structures, our  current  definition  of  the  sub-superior  pattern relationship  can  be  unintuitive  for  certain  user groups.  Further,  we  have  identified  case  when  the sub-superior or potentially conflicting relations are insufficient;  for  example,  some  groups  of  patterns are normally used together to instantiate each other.

## Navigating the pattern collection

With over 200 pattern candidate identified, we have already identified the problem of finding the relevant patterns  for  any  given  situation.  This  problem  is especially apparent to new users of methods using game  design  patterns  and  to  address  this  we  are seeking  various  forms  of  categorizing  patterns  for different  use  areas,  game  themes  and  relations  to

## GAME DESIGN PATTERNS

our structural framework. One especially interesting line  of  research  would  be  to  use  game  design  patterns  to  define  game  genres  and  then  explore  if these patterns are those which are most useful for development or research within those genres.

## The Danger of Stereotyping

Some may object that the use of patterns takes the creativity  out  of  game  design  or  renders  the designers  as  'mere  pattern  cranking  machines' that automatically churn out games. Another common fear is that the use of patterns will lead to a situation where all the games follow the same pattern and fall into stereotypes where nothing new is or can be created. These both stem from confusing the  everyday  meaning  of  pattern  as  something repetitive  with  the  actual  basic  philosophy  of design patterns as introduced by Alexander. In one sense the choice of pattern term might be regarded as a mistake but as the term has clear and firmly  established  meaning  in  several  professional fields we see not necessity for inventing new terminology,  something  that  would  indeed  lesser  the usefulness of the pattern concept as a tool to overcome communication differences in various professions. A more appropriate comparison of the use of patterns is to the artistic endeavor in general: the artist has much better chances to create something novel  when  familiar,  though  not  necessarily  consciously,  of  the  basic  elements  of  her  craft,  be  it painting, composing or scriptwriting.

## CONCLUSION

During our research, we have identified the need for a unified vocabulary and common concepts regarding  games  and  game  design.  Studying  earlier approaches to create common vocabularies, we have concluded that it is appropriate that such a vocabulary  emerge  either  from  terms  and  ideas  that  are already rooted within the gaming community, or that

190

suitable  concepts,  terms  and  methods  are  taken from other disciplines and are carefully adapted to the gaming field without adopting larger conceptual structures.  In  addition,  the  supplements  should focus on the interaction in games, rather than on e.g. narrativity.  Furthermore, they need to be applicable to all kind of games to avoid the risk of being stuck in the developed conventions of digital games. As a solution  to  these  problems,  we  propose  the  use  of patterns.

In line with this, we have created a collection of patterns, primarily based on transforming documented game  mechanics  or  well-defined  concepts  from other research fields. This collection has then been the  basis  for  initial  tested  of  use  areas  for  game design  patterns.  These  tests  have  confirmed  our belief  that  game  design  patterns  are  usable  for analysis, comparison and design of games; thus useful in most aspects within game studies, in turn making them a suitable candidate to serve as a basis of a lingua franca within gaming. We do not believe that the use of game design patterns is the final solution to finding a common language for ludology. However, we believe that many of the characteristics of design patterns will be included in such a language, and that continued work with design patterns will help reveal truths about game and game play until such a language is found.

## ACKNOWLEDGMENTS &amp; CONTACT

We thank our collaborator Tobias Rydenhag as well as  Aki  Järvinen  and  Bernd  Kreimeier  for  valuable discussions. The findings of the work on game design patterns  are  continuously  updated  at  http://www. gamedesignpatterns.org.

## References

1. Alexander, C. et al. (1977). A Pattern Language: Towns, Buildings, Construction. Oxford University Press.
2. Antifakos, S. &amp; Schiele, B. (2002). Unmasking Mister X. Poster at the 6th International Symposium on Wearable Computers, Seattle, Washington, USA.
3. Avedon, E.M. &amp; Sutton-Smith, B. (1971) (eds.): The Study of Games. New York: John Wiley &amp; Sons, Inc.. 1971.
4. Falstein, N (2002). Better By Design: The 400 Project. Game Developer magazine, Vol. 9, Issue 3, March 2002, p. 26 &amp; http://www.theinspiracy.com/400\_project.htm) CHECK
5. Bersak, D., et al. (2001). Intelligent Biofeedback using an Immersive Competitive Environment. Paper at the Designing Ubiquitous Computing Games Workshop at UbiComp 2001, Atlanta, GA, USA.
6. Björk, S., Falk, J., Hansson, R., &amp; Ljungstrand, P. (2001). Pirates! - Using the Physical World as a Game Board. Paper at Interact 2001, IFIP TC.13 Conference on Human-Computer Interaction, July 9-13, Tokyo, Japan.
7. Kreimeier, B., Holopainen, J., &amp; Björk, S. (2003). Game Design Patterns. Lecture Notes from GDC 2003, Game Developers Conference, March 4-8, 2003, San Jose, CA, USA.
8. Björk, S. &amp; Holopainen, J. (2003). Describing Games - An Interaction-Centric Structural Framework. Proceedings of Level Up - 1st international Digital Games Research Conference

191

2003, 4-6 November 2003 University of Utrecht, The Netherlands.

9. Boardgamegeek. http://www.boardgamegeek.com.
10. Borcher, J. (2001). A Pattern Approach to Interaction Design. Wiley.
11. Caillois, R. (2001). Man, Play and Games. University of Illinois Press.
12. Cheok, A. D. et al. (2002). Touch-Space: Mixed Reality Game Space Based on Ubiquitous, Tangible, and Social Computing. Journal of Personal and Ubiquitous Computing, Volume 6, 6th issue: Special issue on Ubiquitous Gaming.
13. Church, D. (1999). Formal Abstract Design Tools. Online article available at www.gamasutra.com.
14. Costikyan, G. (2002). I Have No Words &amp; I Must Design. In Mäyrä, F. (2002) Conference Proceedings of Computer Games and Digital Cultures, pp. 9-33, Tampere University Press.
15. Crawford, C. (1982). The Art of Computer Game Design.
16. Flintham, M., et al. (2003). Where On-Line Meets On-The-Streets: Experiences with Mobile Mixed Reality Games. Proceeding of CHI 2003.
17. Gamma, E., Helm, R., Johnson, R. &amp; Vlissides, J. (2001). Design Patterns - Elements of Reusable Object-Oriented Software. Addison-Wesley, 2001.
18. Gorbet, M., Orth, M., &amp; Ishii, H. (1998). Triangles:

## GAME DESIGN PATTERNS

Tangible Interface for Manipulation and Exploration of Digital Information Technology, Proc. of CHI 1998, ACM Press.

19. Hjelm, S. I. (2003). Research + Design: The Making of Brainball. In Interactions - New visions of Human-Computer Interaction, January + February, 2003, Vol. X.1, pp. 26-34.
20. Huizinga, J. (1986). Homo Ludens: A Study of the Play-element in Culture. Beacon Press.
21. IDSA. (2002). Essential Facts About the Computer and Video Game Industry - 2002 Sales, Demographics and usage Data. Available for download from www.idsa.com.
22. IDSA. (2003). Essential Facts About the Computer and Video Game Industry - 2003 Sales, Demographics and usage Data. Available for download from www.idsa.com.
23. Ishii, H. &amp; Ullmer, B. (1999). Tangible Bits: Towards Seamless Interfaces between People, Bits and Atoms. Proceedings of CHI '97.
24. Järvinen, A (2002). Halo and the Anatomy of the FPS. Volume 2, Issue 1 Game Studies (www.gamestudies.org), July 2002.
25. Järvinen, A. (2003). Games Without Frontiers, Unpublished Ph.D. manuscript.
26. Knizia, R. (1999). Dice Games Properly Explained. Elliot Right Way Books.
27. Kreimeier, B. (2002). The Case For Game Design Patterns. Online publication  available from http://www.gamasutra.com/features/20020313/kreimeier\_03.htm.

192

28. Laurel, B. (1993). Computers as Theatre. Addison-Wesley, Reading, Mass.
29. Lindley, C.A. (2000). A Computational Semiotic Framework for Interactive Cinematic Virtual Worlds. In Proceedings of the First Workshop on Computational Semiotics for New Media, University of Surrey, UK.
30. Lundgren, S. (2002). Joining Bits and Pieces How to make Entirely New Board Games using Embedded Computer Technology. M.Sc. Thesis in Interaction Design at the Department of Computing Science, Chalmers University of Technology.
31. Lundgren, S. &amp; Björk, S. (2003). Game Mechanics: Describing Computer-Augmented Games in Terms of Interaction. Proceeding of TIDSE 2003.
32. Lundgren, S., Falk, J., Björk, S., Holopainen, J. &amp; Åkesson, K-P. (2003). MyTheme. Demonstration at Level Up - 1st international Digital Games Research Conference 2003, 4-6 November 2003 University of Utrecht, The Netherlands.
33. Mandryk, R. L., Maranan, D. S., Inkpen, K. M. (2002). False Prophets: Exploring Hybrid Board/Video Games, Proceedings of CHI 2002. ACM Press.
34. Manovich, L. (2001) The Language of New Media. MIT Press, 2001.
35. Mueller, F., Agamanolis, S., &amp; Picard, R. (2003). Exertion interfaces: sports over a distance for social bonding and fun. In Proceeding of CHI 2003, pp. 561-568, Ft. Lauderdale, Florida, USA

.

36. Murray, J. (1998). Hamlet on the Holodeck: The Future of Narrative in Cyberspace. MIT Press; ISBN: 0262631873.
37. Nack, F. (1996). AUTEUR: The Application of Video Semantics and Theme Representation for Automated Film Editing. Ph.D. Thesis, Lancaster University.
38. Parlett, D. (1999). The Oxford History of Board Games, Oxford University Press.
39. Price, S. (2002). What Sells Where and Why? Lecture notes from Game Developers Conference Europe 2002.
40. Schneider, J. &amp; Kortuem, G. (2001). How to Host a Pervasive Game - Supporting Face-to-Face Interactions in Live-Action Roleplaying. Position paper at the Designing Ubiquitous Computing Games Workshop at UbiComp 2001, Atlanta, GA, USA, September 30, 2001.
41. Spector, W. (1999). Remodeling RPGs for the New Millennium. Online article available at http://www.gamasutra.com/features/game\_desi gn/19990115/remodeling\_01.htm.
42. Spector, W. (2003). Sequels &amp; Adaptations: Design Innovation in a Risk-Averse World. Game Design Keynote at Game Developers' Conference 2003.
43. Smith, H. (2003). Orthogonal Unit Differentiation. Lecture notes from GDC 2003. Presentation available at http://www.gdconf.com/archives/2003/Smith\_H arvey.ppt.
44. Starner, T. et al. (2000). MIND-WARPING: Towards creating a compelling collaborative augmented reality game. Proceedings Intelligent User Interfaces (IUI) 2000 pp 256-259
45. Tamura, H. (2000). What Happens at the Border Between Real and Virtual Worlds The MR Project and Other Research Activities in Japan. Invited talk at ISAR 2000.
46. Tomlinson, B., Blumberg, B., &amp; Nain, D. (2000). Expressive Autonomous Cinematography for Interactive Virtual Environments. In Proc. of Autonomous Agents 2000, ACM Press.
47. Wolf, M.J.P. (2002). Genre and the Video Game. In Wolf, M.J.P (ed.) The Medium of the Video Game, University of Texas Press, 2002. ISBN: 029279150X.
48. Wright, W. (2003). Dynamics for Designers. Presentation at Game Developers Conference 2003. Presentation available at http://www.gdconf.com/ archives/2003/Wright\_Will.ppt.