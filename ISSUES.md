sfPong TODOs
============

* [wip] Suporte a Joystick e Mouse
	- [ ] Arrumar um jeito de serializar e deserializar inputs de Joystick
	- [ ] sensitividade configur�vel
	- [x] deadzone configur�vel
* [x] Unit tests
* [] Sound Fx?
* [] Renomear tags (v0.0.0)
* [x] N�o usar absolute path do ttf
* [x] Menu de sele��o de modo (1 ou 2 jogadores)
* [z] Variavel de vers�o do programa (SFPONG_VERSION)
* [x] BUG: Mousewheel n�o funciona no menu
* [x] tornar posicionamento independende da resolu��o da janela (sf::View)
* [x] refatorar Cfg*
	- CfgPaddle: move.speed e move.acceleration s� s�o usados com input teclado
* [] Melhorar IA
	- [wip] refatorar
	- [] Dificuldade ajustavel
* [x] refatorar menu

Branch TODOs
---
* [] unificar configura��o
	atualmente exite copias de player_input_cfg espalhadas pelo projetos
	criar classa modelando configs, passar ptrs pra quem precisa
* [] separar entidades (paddle, ball) da classe game
* [] encapsular Playarea e pong_court em uma classe
* [] refatorar startup