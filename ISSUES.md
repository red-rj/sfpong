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
* [wip] Melhorar IA
	- [x] refatorar
	- [] Dificuldade ajustavel
* [x] refatorar menu

* [x] unificar configura��o
	atualmente exite copias de player_input_cfg espalhadas pelo projetos
	criar classa modelando configs, passar ptrs pra quem precisa
* [wip] encapsular Playarea e pong_court em uma classe
	parar de depender no valor de playarea
* [] refatorar startup (main)
* [] n�o herdar de shape para player e ball
