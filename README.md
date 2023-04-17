# Projekat: Goldfish and parachute worms
Projekat iz racunarske grafike\
Koriscen je skelet projekta [project base](https://github.com/matf-racunarska-grafika/project_base) kao i materijali iz [LearnOpenGL](https://github.com/matf-racunarska-grafika/LearnOpenGL.git)  repozitorijuma.\
Implementirane su obavezne teme.

Dodatne teme:

Iz grupe A:
- [x] Cubemaps (Skybox)

Iz grupe B:
- [x] HDR (exposure)



# Uputstvo
1. `git clone https://github.com/sntntn/RacunarskaGrafika`
2. CLion -> Open -> path/to/my/project_base
3. Main se nalazi u src/main.cpp
4. ALT+SHIFT+F9 -> project_base -> run


Opis 
---
Cilj je da aktiviramo sve crvice padobrance "parachute worms", i da njima nahranimo nasu zlatnu ribicu.\
Svakim pojedenim crvom ribicin odsjaj postaje sve jaci, sve dok ne pojede i poslednjeg crva.\
Kada pojedemo poslednjeg crva -> nasa misticna kutija ispod ribe postaje zlatni novcic (koji nam omogucava Restart).\
Svaki put kad aktiviramo crvica, aktivira se i njegova prateca zvezda.


---
Komande
---
`ESC` Prekida izvrsavanje projekta

---
Kretanje kamere:\
- `W` `A` `S` `D`\
- Pomeranje misa je rotacija kamere
- Mouse scroll uvelicava sliku 

---

Ukljucivanje/Iskljucivanje:\
`Space` stampa koordinate kamere\
`B` aktiviranje 1. padobranca crvica\
`N` aktiviranje 2. padobranca crvica\
`M` aktiviranje 2. padobranca crvica\
`R` RESTART (omoguceno nam je kada nam izadje novcic)

`P` Blinn\
`O` SpotLight

`1` HDR\
`F` BLOOM

`Q` Exposure +\
`E` Exposure -
---
ImGui:\
`I` otvaranje/zatvaranje ImGui prozora

## Resursi:
[coin](https://rigmodels.com/model.php?view=Coin-3d-model__I89O58TBZ353I4X9ANHTRFF5K&searchkeyword=coin&manualsearch=1)\
[padobran](https://rigmodels.com/model.php?view=Parachute_Worm-3d-model__Q41SJLO72KVS0UJF2D1DM2OXO&searchkeyword=parachute&manualsearch=1)\
[Zvezda](https://rigmodels.com/model.php?view=Star_Fish-3d-model__O5T6WV158SFXN8YLU5HYHWW1Q&searchkeyword=star&manualsearch=1)\
[Riba](https://rigmodels.com/model.php?view=Fish-3d-model__L5GWQCNY48U447XE7EQDWOFVC&searchkeyword=fish&manualsearch=1)


### [Youtube video prezentacija bez HDR](https://www.youtube.com/watch?v=PqqQ0k7qxVU)
### [Youtube video prezentacija sa HDR](https://www.youtube.com/watch?v=22H2bCjXMRk)
### [Youtube video prezentacija HDR i BLOOM](https://www.youtube.com/watch?v=DAeUchHD-hc)

Autor: Vukasin Markovic 312/2019


