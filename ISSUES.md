sfPong TODOs
============

* [wip] Suporte a Joystick e Mouse
	- [ ] Arrumar um jeito de serializar e deserializar inputs de Joystick
	- [ ] sensitividade configurável
	- [x] deadzone configurável
* [x] Unit tests
* [] Sound Fx?
* [] Renomear tags (v0.0.0)
* [x] Não usar absolute path do ttf
* [x] Menu de seleção de modo (1 ou 2 jogadores)
* [z] Variavel de versão do programa (SFPONG_VERSION)
* [x] BUG: Mousewheel não funciona no menu
* [x] tornar posicionamento independende da resolução da janela (sf::View)
* [x] refatorar Cfg*
	- CfgPaddle: move.speed e move.acceleration só são usados com input teclado
* [] Melhorar IA
	- [wip] refatorar
	- [] Dificuldade ajustavel
* [x] refatorar menu

Branch TODOs
---
* [] unificar configuração
	atualmente exite copias de player_input_cfg espalhadas pelo projetos
	criar classa modelando configs, passar ptrs pra quem precisa
* [] separar entidades (paddle, ball) da classe game
* [] encapsular Playarea e pong_court em uma classe
* [] refatorar startup