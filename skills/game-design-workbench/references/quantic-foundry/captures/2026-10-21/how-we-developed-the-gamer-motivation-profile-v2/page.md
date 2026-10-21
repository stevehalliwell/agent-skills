[Skip to content](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/#content)
[ ![Quantic Foundry Logo](https://quanticfoundry.com/wp-content/uploads/2015/05/logo-50.png) ](https://quanticfoundry.com/)
Search for:
[](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/)
  * [Motivation Model](https://quanticfoundry.com/#motivation-model)
  * [Audience Reports](https://quanticfoundry.com/reports)
  * [Team](https://quanticfoundry.com/#team)
  * [Contact](https://quanticfoundry.com/#contact)
  * [Blog](https://quanticfoundry.com/blog/)
  * [Apps](http://apps.quanticfoundry.com)
  * [](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/ "Search")

Go to...
  * [Motivation Model](https://quanticfoundry.com/#motivation-model)
  * [Audience Reports](https://quanticfoundry.com/reports)
  * [Team](https://quanticfoundry.com/#team)
  * [Contact](https://quanticfoundry.com/#contact)
  * [Blog](https://quanticfoundry.com/blog/)
  * [Apps](http://apps.quanticfoundry.com)


[Previous](https://quanticfoundry.com/2015/07/20/over-30000-gamers-have-taken-our-gamer-motivation-profile/) [Next](https://quanticfoundry.com/2015/08/11/most-popular-games-by-gaming-motivations/)
  * ![](https://i0.wp.com/quanticfoundry.com/wp-content/uploads/2015/07/photodune-9235903-game-m-16x9.jpg?fit=1280%2C721&ssl=1)


# How We Developed The Gamer Motivation Profile v2
By [Nick Yee](https://quanticfoundry.com/author/nickyee/)|2016-10-17T20:02:35-07:00July 20th, 2015|[Analytics](https://quanticfoundry.com/category/analytics/), [Video Games](https://quanticfoundry.com/category/video-games/)|[19 Comments](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/#comments)
> **The super short version** : We validated and refined our motivations model based on data from 30,000 gamers around the world.
> If you’re a gamer and haven’t taken the [Gamer Motivation Profile](http://apps.quanticfoundry.com/lab/10) yet, consider doing so before reading about the model.
We developed the Gamer Motivation Profile such that data collected over time could be used to test new motivations, try out new features, and refine the backend algorithms. Since we launched the profile tool, [over 30,000 gamers](https://quanticfoundry.com/2015/07/20/over-30000-gamers-have-taken-our-gamer-motivation-profile) around the world have taken it. The size and geographic scope of this data has allowed us to validate and refine our motivation model.
The initial version of the Gamer Motivation Profile allowed us to collect large samples of English-speaking gamers from many geographic regions ([see detailed sample notes](https://quanticfoundry.com/notes-on-phase-2-motivation-profile-sample/)):
  * US + Canada: 6,222
  * Brazil: 6,044
  * Indonesia: 6,000
  * Philippines: 3,198
  * Singapore: 2,316
  * EU: 2,004
  * Australia: 660
  * East Asia: 510
  * (long tail of other countries)


We are continually testing out new items in the profile tool. Thus, interspersed within the survey inventory are additional items being tested to improve existing scales or to explore factors that are suggested by theory or gamer input. As we did in developing the [original model](https://quanticfoundry.com/2015/06/18/how-we-created-the-gamer-motivation-profile/), we conducted a factor analysis of the inventory items. We conducted a factor analysis separately on the US + Canada, Indonesia, and EU data. The number of factors that emerged and the factor composition across these 3 regions were identical. The same results also emerged when a factor analysis was applied to the full data set.
> The number of factors that emerged and the factor composition across these 3 regions were identical.
To be clear, this doesn’t mean that there were no geographical differences in motivations, only that the factor structure was the same across regions. Or put another way, everyone recognizes that there is a set of cooked dishes which are spicy even though preference and tolerance for spicy foods vary a great deal from person to person.
### Visualizing How The Factors are Related
We identified 12 motivation factors. To understand and visualize how these factors cluster and relate to each other, we used hierarchical clustering. In brief, this clustering method starts every variable on its own, and then merges the most similar variables together into clusters; the algorithm continues merging clusters until everything is in one cluster. The visual output is called a dendrogram and graphically shows how our motivation factors are related: the earlier that two variables merge, the more closely related they are.
[![dendrogram](https://i0.wp.com/quanticfoundry.com/wp-content/uploads/2015/07/dendrogram.png?resize=650%2C377&ssl=1)](https://i0.wp.com/quanticfoundry.com/wp-content/uploads/2015/07/dendrogram.png?ssl=1)
The dendrogram reveals an interesting hierarchical pattern among the 12 motivations. First, there are 6 pairs of motivations that are closely related. These are largely the same clusters as in the [earlier version of the model](https://quanticfoundry.com/2015/06/18/how-we-created-the-gamer-motivation-profile/), with the addition of the Discovery factor. And then these clusters fall into 3 high-level groups.
### Our Gamer Motivations Framework
First, we’ll describe the 12 motivation factors and how they cluster together.
**Immersion:** Gamers with high Immersion scores want games with interesting narratives, settings, and customization options so they can be deeply immersed in the alternate worlds created by games. Gamers with low Immersion scores are more grounded in the gameplay mechanics and care less about the narrative experiences that games offer. Immersion is composed of 3 underlying motivations:
  * Fantasy: The desire to become someone else, somewhere else.
  * Story: The importance of an elaborate storyline and interesting characters.


**Creativity:** Gamers with high Creativity scores are constantly experimenting with their game worlds and tailoring them with their own designs and customizations. Gamers with low Creativity scores are more practical in their gaming style and accept their game worlds as they are.
  * Design: The appeal of expression and deep customization.
  * Discovery: The desire to explore, tinker, and experiment with the game world.


**Action:** Gamers with high Action scores are aggressive and like to jump in the fray and be surrounded by dramatic visuals and effects. Gamers with low Action scores prefer slower-paced games with calmer settings. Action is composed of two underlying motivations:
  * Destruction: The enjoyment of chaos, mayhem, guns, and explosives.
  * Excitement: The enjoyment of games that are fast-paced, intense, and provide an adrenaline rush.


**Social:** Gamers with high Social scores enjoy interacting with other players, often regardless of whether they are collaborating or competing with them. Gamers with low Social scores prefer solo gaming experiences where they can be independent. Social is composed of two underlying motivations:
  * Competition: The enjoyment of competition with other players (duels or matches).
  * Community: The enjoyment of interacting and collaborating with other players.


**Mastery:** Gamers with high Mastery scores like challenging gaming experiences with strategic depth and complexity. Gamers with low Mastery scores enjoy being spontaneous when gaming and prefer games that are accessible and forgiving when mistakes are made. Mastery is composed of two underlying motivations:
  * Challenge: The preference for games of skill and enjoyment of overcoming difficult challenges.
  * Strategy: The enjoyment of games that require careful decision-making and strategic thinking.


**Achievement:** Gamers with high Achievement scores are driven to accrue power, rare items, and collectibles, even if this means grinding for a while. Gamers with low Achievement scores have a relaxed attitude towards in-game achievements and don’t worry too much about their scores or progress in the game. Achievement is composed of two underlying motivations:
  * Completion: The desire to complete every mission, get every collectible, and discover hidden things.
  * Power: The importance of becoming powerful within the context of the game world.


### What’s Changed from Version 1.0
There are two main changes from the earlier motivation framework:
  * We tested and found inventory items for a Discovery factor—experimenting with the game world in different ways and constantly asking “what if?” This Discovery factory was most closely related to the existing Design factor (originally named Customization) and together form the Creativity cluster.
  * Based on user feedback to the profile reporting, we’ve renamed the “Strategy” cluster. The problem was that gamers reacted negatively to seeing low/average scores on “Strategy” when it was dragged down by the importance of challenging gameplay. We’ve relabeled the factors to clarify their underlying meaning. So the cluster label is now “Mastery”, which includes the “Strategy” and “Challenge” factors.


### Revisiting The Hierarchy
The factor analysis and hierarchical clustering results not only enumerate gamer motivations, they also show how the motivations are related to each other. One key challenge in studying gaming motivations is that it’s relatively easy to brainstorm and list potential motivations, but understanding the structure underlying those motivations, how they relate to each other, and how to reliably measure them require large amounts of data.
> The factor analysis and hierarchical clustering results not only enumerate gamer motivations, they also show how the motivations are related to each other.
The hierarchical clustering suggests that the 12 motivation factors fall into 3 high-level groupings:
  * The Immersion-Exploration branch covers different ways of relating to the story and design of the game world, whether via the narrative, the characters, or exploring and customizing the game world.
  * The Achievement-Mastery branch covers different ways or progressing through and attaining power within the construct of the game world, whether this is leveling up, completing all its missions, or gaining mastery through practice.
  * The Action-Social branch covers more energetic and gregarious modes of gameplay, seeking out arousing gaming experiences whether this is from playing with other people, intense gameplay, or dramatic destruction.


The first two of these high-level groupings make intuitive sense. The Action-Social branch may be the least intuitive branch, but in hindsight, it is predicted by the [Big Five personality model](https://en.wikipedia.org/wiki/Big_Five_personality_traits). The Big Five model is the current gold standard in describing and measuring personality in academic psychology. In this personality framework, studies have found that [Extraversion](https://en.wikipedia.org/wiki/Big_Five_personality_traits#Extraversion) bundles together traits related to gregariousness, excitement-seeking, assertiveness, and cheerfulness—an interesting mix of traits that is broader than the popular science description of Extraversion. The Action-Social branch seems to reflect this combination of Extraversion traits.
### The Updated Profile
We’ve updated the [profile tool](https://apps.quanticfoundry.com/lab/) to reflect these changes. In addition to adding the Discovery factor and revising the clusters, we’ve revised the inventory item weights and overall norms based on the larger sample of data.
Gamers who have previously completed the original version of the tool will have a link to their legacy profile and will also see a link to take the updated profile tool. All new users will simply see a link to the current version of the tool (whether they land on the Quantic Lab page directly or via a shared profile report).
#### Share This Story, Choose Your Platform!
[Facebook](https://www.facebook.com/sharer.php?u=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&t=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2)[X](https://x.com/intent/post?url=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&text=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2)[Reddit](https://reddit.com/submit?url=https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/&title=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2)[LinkedIn](https://www.linkedin.com/shareArticle?mini=true&url=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&title=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2&summary=The%20super%20short%20version%3A%C2%A0We%20validated%20and%20refined%20our%20motivations%20model%20based%20on%20data%20from%2030%2C000%20gamers%20around%20the%20world.%0D%0A%0D%0AIf%20you%E2%80%99re%20a%20gamer%20and%20haven%E2%80%99t%20taken%20the%20Gamer%20Motivation%20Profile%20yet%2C%20consider%20doing%20so%20before%20reading%20about%20the%20model.%0D%0AWe%20develo)[Tumblr](https://www.tumblr.com/share/link?url=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&name=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2&description=The%20super%20short%20version%3A%C2%A0We%20validated%20and%20refined%20our%20motivations%20model%20based%20on%20data%20from%2030%2C000%20gamers%20around%20the%20world.%0D%0A%0D%0AIf%20you%E2%80%99re%20a%20gamer%20and%20haven%E2%80%99t%20taken%20the%20Gamer%20Motivation%20Profile%20yet%2C%20consider%20doing%20so%20before%20reading%20about%20the%20model.%0D%0AWe%20developed%20the%20Gamer%20Motivation%20Profile%20such%20that%20data%20collected%20over%20time%20could%20be%20used%20to%20test)[Pinterest](https://pinterest.com/pin/create/button/?url=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&description=The%20super%20short%20version%3A%C2%A0We%20validated%20and%20refined%20our%20motivations%20model%20based%20on%20data%20from%2030%2C000%20gamers%20around%20the%20world.%0D%0A%0D%0AIf%20you%E2%80%99re%20a%20gamer%20and%20haven%E2%80%99t%20taken%20the%20Gamer%20Motivation%20Profile%20yet%2C%20consider%20doing%20so%20before%20reading%20about%20the%20model.%0D%0AWe%20developed%20the%20Gamer%20Motivation%20Profile%20such%20that%20data%20collected%20over%20time%20could%20be%20used%20to%20test&media=https%3A%2F%2Fi0.wp.com%2Fquanticfoundry.com%2Fwp-content%2Fuploads%2F2015%2F07%2Fphotodune-9235903-game-m-16x9.jpg%3Ffit%3D1280%252C721%26ssl%3D1)[Vk](https://vk.com/share.php?url=https%3A%2F%2Fquanticfoundry.com%2F2015%2F07%2F20%2Fhow-we-developed-the-gamer-motivation-profile-v2%2F&title=How%20We%20Developed%20The%20Gamer%20Motivation%20Profile%20v2&description=The%20super%20short%20version%3A%C2%A0We%20validated%20and%20refined%20our%20motivations%20model%20based%20on%20data%20from%2030%2C000%20gamers%20around%20the%20world.%0D%0A%0D%0AIf%20you%E2%80%99re%20a%20gamer%20and%20haven%E2%80%99t%20taken%20the%20Gamer%20Motivation%20Profile%20yet%2C%20consider%20doing%20so%20before%20reading%20about%20the%20model.%0D%0AWe%20developed%20the%20Gamer%20Motivation%20Profile%20such%20that%20data%20collected%20over%20time%20could%20be%20used%20to%20test)Email
##  About the Author: [Nick Yee](https://quanticfoundry.com/author/nickyee/)
![](https://secure.gravatar.com/avatar/b33fb34ca86000571cecc63b74546c85a1b7e30abfb0b5bc1f6023262b6c9a91?s=72&d=mm&r=g)
Nick is the co-founder and analytics lead of Quantic Foundry. He combines social science and data science to understand gamer behavior in large-scale game data. 
##  19 Comments 
  1. **Over 30,000 Gamers Have Taken Our Gamer Motivation Profile - Quantic Foundry** July 20, 2015 at 10:55 pm 
[…] Previous Next […]
  2. ![](https://secure.gravatar.com/avatar/71a4fdb040a1ae7395fa45deaa8e2fb98aa42c06af6700d776438500113a3878?s=54&d=mm&r=g)
**Bart Stewart** July 22, 2015 at 12:19 am 
Hi, Nick.
After taking the previous version of the inventory, I provided some detailed notes on an alternative organization for the factors that stood out to you. It looks like a few of those were considered, which I think does improve your model, although of course I know I’m not the only person who must have offered constructive suggestions.
The new version actually does, I think, now show a closer relationship to gamer data organized by the (equally self-reporting) Myers-Briggs/Keirsey models and their gaming analogue in the Bartle Types:
Fantasy + Story = Socializer (identity-seeking, interested in forming and nurturing relationships among characters)  
Design + Discovery = Explorer (knowledge-seeking, interested in creatively comprehending game world systems)  
Destruction + Excitement + Community + Competition = Manipulator [Bartle’s “Killer”] (excitement-seeking, interested in the entertainment that comes from manipulating people and the game environment)  
Challenge + Strategy + Completion + Power = Achiever (security-seeking, interested in maximizing the accumulation of status tokens versus other players)
The relative numbers of factors associated with each of the four basic motivations are — given 30,000 respondents — closely in line with the numbers of each of the four temperaments in the general population. In other words, it’s not surprising to see twice as many factors associated with Achievers and Manipulators as with Explorers and Socializers. (Although I’d bet that your numbers would support grouping Challenge with the four Achiever-ish factors, for an even closer fit with the distribution of the temperaments.)
I expect that you’re still unlikely to find this perspective to be a confirmation that your revised model is an accurate reflection of general and gamer preferences. ;) Still, I found it interesting and wanted to mention it in case you might find it of some interest as well.
Good luck with the new project!
  3. ![](https://secure.gravatar.com/avatar/b33fb34ca86000571cecc63b74546c85a1b7e30abfb0b5bc1f6023262b6c9a91?s=54&d=mm&r=g)
**Nick Yee** July 22, 2015 at 9:10 am 
Thanks for the suggestions, Bart! And glad to hear you think the revised model is an improvement.
  4. **Most Popular Games by Gaming Motivations - Quantic Foundry** August 11, 2015 at 3:55 pm 
[…] Previous […]
  5. **Gender and Age Differences in Gaming Motivations** August 28, 2015 at 10:55 am 
[…] following findings are based on data from 107,100 gamers who have taken the Gamer Motivation Profile since late […]
  6. **Game Audience Profiles - Quantic Foundry** September 4, 2015 at 10:09 am 
[…] we started developing the Gamer Motivation Profile, we wanted to make sure we had a way of debugging any potential issues with the motivation scores. […]
  7. ![](https://secure.gravatar.com/avatar/5bc67f25a944a1e490f33002dc66155363b7ea9db1b954a3fbf439919dd962b4?s=54&d=mm&r=g)
**Seb** October 29, 2015 at 3:03 am 
completion needs changing to competition in the social category, typo me thinks :)
     * ![](https://secure.gravatar.com/avatar/b33fb34ca86000571cecc63b74546c85a1b7e30abfb0b5bc1f6023262b6c9a91?s=54&d=mm&r=g)
**Nick Yee** October 29, 2015 at 10:28 am 
Thanks for catching that! Fixed it.
  8. **The Gamer Motivation Model in Handy Reference Chart and Slides - Quantic Foundry** December 15, 2015 at 11:15 am 
[…] A total of over 140,000 gamers worldwide have now completed the Gamer Motivation Profile. Statistical analysis of how motivations cluster together is consistent with what we reported earlier. […]
  9. ![](https://secure.gravatar.com/avatar/e7bf243466ea5ca3f81acb98bc3a686a32ddbfb07e089077e5af7670ba157f61?s=54&d=mm&r=g)
**Nat** December 16, 2015 at 9:13 pm 
I was really excited to try this out, and see how the model analyzes people!
Then I got to question 3. 
So, this isn’t a “gamer model”, it’s a “videogamer model”. That’s too bad—I suspect you’d find a lot of overlap and really interesting data if you collected from people who play more varieties of games. I’d be particularly interested to see if there’s any skew between people who prefer different game types (do boardgamers tend towards Community or Strategy more or less than video gamers? etc.). (My answer to question 5, BTW, would be “tabletop”—it’s RPGs and sometimes boardgames for me. Basically never touch computer/console/video games, and prefer cooperative to competitive boardgames.)
  10. **New study analyzes gamer motives | BYTEbsu** February 11, 2016 at 9:01 am 
[…] Source: Quantic Foundry […]
  11. **Revisiting the Strategy Genre Map: Age, Audience Homogeneity, and the Lasso Effect** March 23, 2016 at 10:35 am 
[…] The 12 motivations that are measured in our model were identified via statistical analysis of how gaming motivations cluster together (based on data from 30,000 […]
  12. ![](https://secure.gravatar.com/avatar/1b25094f380521e41e6f941c354e82393ed42ca9b728214e5fd6961fafd31948?s=54&d=mm&r=g)
**investir dans l'immobilier avec 10 000 euros** June 10, 2016 at 3:31 pm 
They allow you to access the information conveniently from your own computer.
Think about attending group meetings and join forums.  
There are two very major differences between investing in stock and investing in real estate.
  13. ![](https://secure.gravatar.com/avatar/38d0957c4c74b56fbf9396d462d78ad70dc79f136ef506e7c60686176eb7f0be?s=54&d=mm&r=g)
**summoners war guide** June 11, 2016 at 1:42 pm 
There are many reasons people like to play co-op or custom against bots:.
Master the art of the shadow, and strike from darkness as the  
sexy yet deadly champion Akali. MT: You recently partnered with Cryptozoic Entertainment.
  14. ![](https://secure.gravatar.com/avatar/3d41b3a27cb01bbee6dbf7b29e567c4042270f82a0322b4315405e69d6ebedfc?s=54&d=mm&r=g)
**Buy quality dressing table online** June 16, 2016 at 5:56 pm 
Thank you for the auspicious writeup. It if truth be told was once a enjoyment account it.  
Look advanced to more added agreeable from you! By the way, how can we be in contact?
  15. ![](https://secure.gravatar.com/avatar/d13749d5c8b87fc85959418e8620edfc523fb7e97365d3192ac146f89dbe2521?s=54&d=mm&r=g)
**Rozliczenie Podatku Holandia Kalkulator 2013** June 21, 2016 at 11:48 am 
A motivating discussion is definitely worth comment.
I do think that you need to publish more on this topic,  
it might not be a taboo matter but usually folks don’t talk about such subjects.
To the next! Best wishes!!
  16. ![](https://secure.gravatar.com/avatar/ae45e227180ebe81c1fd3308fe40504f297c750a1447f8188d4f45bb585dc7ff?s=54&d=mm&r=g)
**UoN** July 6, 2016 at 4:02 pm 
They may be broadly qualified to be a merit scholarship, need based, student based and career based  
scholarships. Along while using above, college can also be  
a possibility to explore what you would like to do in life.  
It is actually beneficial to also speak to parents  
of students whorrrre doing their undergraduate studies abroad to learn why they chose this path, and  
what they have to did to obtain there.
  17. ![](https://secure.gravatar.com/avatar/999d5e65cdf53a640d30ceb79f40398583263554846d4edcc96dd2f39f6f288b?s=54&d=mm&r=g)
**IDO Satoshi** December 14, 2016 at 1:35 am 
It was a wonderful theory and I was impressed.  
But in my opinion there are two motivations out of your framework.
1) Benefit  
Expectation for the profit brought about in real life  
Ex) “Brain Age”, serious games
2) Love  
Fun to control or influence what the player likes  
Ex) sports games, driving games  
* This is close to Fantasy, but not necessarily immersed in something.
In order to complete the Game Recommender Engine, I think that it is necessary to incorporate the above motivation.  
What do you think?
  18. ![](https://secure.gravatar.com/avatar/52f8b3997228992780244c8533ecf8a64789fac1b2d757c2016f37b561848383?s=54&d=mm&r=g)
**seo belgie** June 3, 2019 at 9:42 am 
Nice post! Thank you.


Comments are closed.
Search for:
#### Categories
  * [Analytics](https://quanticfoundry.com/category/analytics/)
  * [Board Games](https://quanticfoundry.com/category/board-games/)
  * [Company News](https://quanticfoundry.com/category/company-news/)
  * [Methodology](https://quanticfoundry.com/category/methodology/)
  * [Survey Findings](https://quanticfoundry.com/category/survey-findings/)
  * [Video Games](https://quanticfoundry.com/category/video-games/)


#### Recent Posts
  * [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/)
  * [Beyond the Headline: Unpacking the Reported Drop in Conscientiousness](https://quanticfoundry.com/2025/08/19/conscientiousness-drop/)
  * [Fabricated Data at Scale: The State of Survey Panels & AI Data](https://quanticfoundry.com/2025/06/17/panel-data-ai/)
  * [Gamers Have Become Less Interested in Strategic Thinking and Planning](https://quanticfoundry.com/2024/05/21/strategy-decline/)
  * [Beam Me Up: The Demographic and Motivational Appeal of Sci-Fi](https://quanticfoundry.com/2023/11/07/sci-fi/)


#### Recent Comments
  * az888id on [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/#comment-265403)
  * postid-1051112 on [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/#comment-265399)
  * Can we stop blindly accusing indie games of using generative AI? – Luna's Gaming Log on [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/#comment-264105)
  * Antti Lehtonen on [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/#comment-264016)
  * GenAge on [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/#comment-263685)


#### Quantic Foundry
Quantic Foundry is a market research company focused on gamer motivation. We combine social science with data science to understand what drives gamers.
#### Recent Blog Posts
  * [Gamers Are Overwhelmingly Negative About Gen AI in Video Games, but Attitudes Vary by Gender, Age, and Gaming Motivations.](https://quanticfoundry.com/2025/12/18/gen-ai/)
  * [Beyond the Headline: Unpacking the Reported Drop in Conscientiousness](https://quanticfoundry.com/2025/08/19/conscientiousness-drop/)
  * [Fabricated Data at Scale: The State of Survey Panels & AI Data](https://quanticfoundry.com/2025/06/17/panel-data-ai/)
  * [Gamers Have Become Less Interested in Strategic Thinking and Planning](https://quanticfoundry.com/2024/05/21/strategy-decline/)


#### Get Social
[](https://www.facebook.com/quanticfoundry) [](https://twitter.com/quanticfoundry)
#### Learn More
[Privacy Policy](https://quanticfoundry.com/privacy-policy)  
[Terms of Use](https://quanticfoundry.com/terms-of-use)  
[FAQ](https://quanticfoundry.com/faq/)
Email: team@quanticfoundry.com
Copyright 2015-2025 Quantic Foundry, LLC. 
[Page load link](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/) [ Go to Top ](https://quanticfoundry.com/2015/07/20/how-we-developed-the-gamer-motivation-profile-v2/) ![](https://pixel.wp.com/g.gif?v=ext&blog=186092061&post=764&tz=-7&srv=quanticfoundry.com&j=1%3A16.2&host=quanticfoundry.com&ref=&fcp=832&rand=0.05915082299330143)
