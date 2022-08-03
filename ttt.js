//
// Author: Krzysztof Cieslak
// Repository: https://github.com/krcs/tic-tac-toe
// License: MIT
//

class TTTAI {
    #ai = null;
    player1 = 0;
    player2 = 0;

    static #importObj = { 
        env: {
            stackSave: n => 256, 
            stackRestore: n => 0,
            //__stack_pointer: new WebAssembly.Global({
            //    value: 'i32', 
            //    mutable: true
            //}, 0),
            __table_base: 0,
            __indirect_function_table: new WebAssembly.Table({
                initial: 0,
                maximum: 65536, 
                element: 'anyfunc'
            }),
            memory: new WebAssembly.Memory({
                'initial': 256,
                'maximum': 65536
            }),
            __memory_base: 0,
            //table: new WebAssembly.Table({ 
            //    initial: 0, 
            //    maximum: 65536, 
            //    element: 'anyfunc'
            //})
        } 
    };

    constructor(wasm) {
        this.wasm = wasm;
    }

    async initialize() {
        this.#ai = await this.#initTTTAI();
    }

    async #initTTTAI() {
        const response = await fetch(this.wasm);
        const bytes = await response.arrayBuffer();
        const instance = await WebAssembly.instantiate(bytes, TTTAI.#importObj);
        return { 
            generate_move : instance.instance.exports.generate_move,
            check_player : instance.instance.exports.check_player
        }
    };
    
    reset() {
        this.player1 = 0;
        this.player2 = 0;
    }

    generateMoveForPlayer1() {
        return this.#ai.generate_move(1, this.player1, this.player2);
    }

    generateMoveForPlayer2() {
        return this.#ai.generate_move(-1, this.player1, this.player2);
    }

    check() {
        if (this.#ai.check_player(this.player1) > 0)
            return 1;
        else if (this.#ai.check_player(this.player2) > 0)
            return 2;
        else if (((this.player1 | this.player2) & 511) == 511)
            return 3;
        return 0;
    }
}

class Game {
    #board = null;
    #tttai = null;
    #isGameLocked = true;
    info = null;
    static BITS = [1,2,4,8,16,32,64,128,256];

    static random01() {
        return Math.random() < 0.5 ? 0 : 1;
    }

    constructor(tttai) {
        this.#tttai = tttai;
        this.#board = [];
    }

    setInfoElement(element) {
        this.info = element;
    }

    setBoardElements(elements) {
        this.#board = elements;
        let callback = null;
        for (let idx in this.#board) { 
            callback = this.#eventCallback.bind(this, idx);
            this.#board[idx].addEventListener("click", callback);
        }
    }

    #eventCallback(idx) {
        if (this.#isGameLocked)
            return;

        const move = 1 << idx;
        if ((this.#tttai.player1 & move) === 0 && 
            (this.#tttai.player2 & move) === 0) {
            this.#tttai.player1 = this.#tttai.player1 | move;
            this.#board[idx].innerHTML = this.human;
            this.info.innerHTML = "Computer's move.";
            this.#computersMove();
        }

        this.#isGameLocked = this.#checkGameState();
    }

    #computersMove() {
        if (this.#isGameLocked)
            return;

        const move = this.#tttai.generateMoveForPlayer2();
        if (move > 0) {
            this.#tttai.player2 = this.#tttai.player2 | move;
            this.#board[Game.BITS.indexOf(move)].innerHTML = this.computer;
            this.info.innerHTML = "Your move.";
        }

        this.#isGameLocked = this.#checkGameState();
    }

    #checkGameState() {
        const check = this.#tttai.check();
        switch (check) {
            case 1:
                this.info.innerHTML = "You have won!";
                return true;
            case 2:
                this.info.innerHTML = "The Computer has won!";
                return true;
            case 3:
                this.info.innerHTML = "Draw."
                return true;
        }
        return false;
    }

    restart() {
        const s = Game.random01();
        if (s === 0) {
            this.human = 'O';
            this.computer = 'X';
        } else {
            this.human = 'X';
            this.computer = 'O';
        }
        this.#clearBoard();
        this.#tttai.reset();
        const firstMove = Game.random01();
        if (firstMove === 1) {
            this.#computersMove();
        }
        this.info.innerHTML = "Your move.";
        this.#isGameLocked = false;
    }

    #clearBoard() {
        for (let element of this.#board)
            element.innerHTML = "";
    }
}

const main = async () => {
    const tttai = new TTTAI("ttt.wasm");
    await tttai.initialize();

    const startButton = document.getElementById("start");
    const game = new Game(tttai);
    game.setInfoElement(document.getElementById("info"));
    game.setBoardElements([
        document.getElementById("c001"),
        document.getElementById("c002"),
        document.getElementById("c004"),
        document.getElementById("c008"),
        document.getElementById("c016"),
        document.getElementById("c032"),
        document.getElementById("c064"),
        document.getElementById("c128"),
        document.getElementById("c256"),
    ]);
    startButton.addEventListener("click", () => {
        game.restart();
    });
}

(async () => {
    await main();
})();
