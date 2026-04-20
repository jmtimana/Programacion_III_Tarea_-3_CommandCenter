#include <iostream>
#include <string>
#include <functional>
#include <list>
#include <map>
#include <cmath>

class Entity {
    private:

    //ATRIBUTOS
    int posicion_x, posicion_y;
    int vida_energia;
    std::string nombre;
    int nivel;
    std::string recursos;

    public:

    //Constructor
    Entity(std::string nombre, std::string recursos): posicion_x(0), posicion_y(0), vida_energia(100),
    nombre(nombre), nivel(0), recursos(recursos) {};

    //METODOS

    //Metodo para mover al personaje
    void move(int dx, int dy) {
        posicion_x += dx;
        posicion_y += dy;
        nivel += static_cast<int>(std::sqrt(dx*dx + dy*dy));  //Interactua con el nivel
    };

    //Metodo para curar al personaje
    void heal(int amount) {
        vida_energia += amount;
        nivel += amount * 2;
    };

    //Metodo para dañar al personaje
    void damage(int amount) {
        vida_energia -= amount;
    };

    // Metodo para agregar recursos
    void addResource(const std::string& nuevo) {
        if (recursos.empty()) {
            recursos = nuevo;
        }
        else {
            recursos += ", " + nuevo;
        }
        nivel += 5;
    }

    //Metodo para resetear al personaje
    void reset() {
        vida_energia = 100; posicion_x = 0;
        posicion_y = 0; nivel = 0;
    };

    //Metodo para mostrar datos
    void status()const {
        std::cout << "\nEl jugador " << nombre << std::endl;
        std::cout << "Tiene nivel " << nivel << std::endl;
        std::cout << "Tiene vida " << vida_energia << std::endl;
        std::cout << "Tiene recursos " << recursos << std::endl;
        std::cout << "Y esta en la posicion X: " << posicion_x << ", Y: " << posicion_y << "\n\n";
    };

    //Metodo para obtener datos para el historial
    std::string obtener_status_para_historial() const {
        return "Jugador: " + nombre + " | Nivel: " + std::to_string(nivel) + " | Vida: " + std::to_string(vida_energia) +
            " | Posicion(" + std::to_string(posicion_x) + "," + std::to_string(posicion_y) + ")" + " | Recursos: " + recursos;
    }
};

//Definir el tipo Command
using Command = std::function<void(const std::list<std::string>&)>;

class CommandCenter {
    private:
    //Atributos de ComandCenter
    std::map<std::string, Command> commands;    //Mapa de comandos
    std::list<std::string> history;             //Historial para guardar datos
    Entity& entity;

    public:

    //Constructor
    CommandCenter(Entity& e):entity(e) {}      //Constructor para poder modificar el Entity

    //METODOS

    //Para registrar comandos al map
    void registerCommand(const std::string& name, Command comando) {
        commands[name] = comando;               //Para interactuar con el comando, registrar o actualizar
    }

    //Para ejecutar un comando
    bool execute(const std::string& name, const std::list<std::string>& args, bool guardar_historial = true) {
        //Un bool para que en caso de que no exista un comando se cancele en comandos compuestos
        std::map<std::string, Command>::iterator it = commands.find(name);
        if (it != commands.end()) {
            std::string historial_antes = entity.obtener_status_para_historial();
            it->second(args);
            std::string historial_despues = entity.obtener_status_para_historial();

            if (guardar_historial) {
                std::string datos =
                    "Comando: " + name + "\nANTES: " + historial_antes + "\nDESPUES: " + historial_despues + "\n";
                history.push_back(datos);
            }
            return true;
        }

        else {
            std::cout << "Error: comando " << name << " no existe\n" << std::endl;
            return false;
        }
    }

    //Historial de comandos
    void mostrarhistorial() {
        std::cout << "\nHistorial de comandos:\n";
        int i = 1;
        for (std::list<std::string>::iterator it = history.begin(); it != history.end(); ++it) {
            std::cout << "---- [" << i << "] ----\n";
            std::cout << *it << std::endl;
            i++;
        }
    }

    //Eliminar un comando
    void removeCommand(const std::string& name) {
        std::map<std::string, Command>::iterator it = commands.find(name);
        if (it == commands.end()) {
            std::cout << "Error: comando no encontrado" << std::endl;
            return;
        }
        std::cout << "Comando eliminado: " << name << std::endl;
        commands.erase(it);
    };

    //Comandos compuestos (Macro Commands)
    std::map<std::string, std::list<std::pair<std::string, std::list<std::string>>>> macros;  //map para comandos compuestos

    //Para registrar comandos compuestos en el map
    void registerMacro(const std::string& name, const std::list<std::pair<std::string, std::list<std::string>>>& commands) {
        macros[name] = commands;
    }

    void executeMacro(const std::string& name) {
        std::map<std::string,
        std::list<std::pair<std::string, std::list<std::string>>>>::iterator it;

        it = macros.find(name);

        if (it == macros.end()) {
            std::cout << "Error: macro " << name << " no existe\n";
            return;
        }

        std::string historial_antes = entity.obtener_status_para_historial();

        std::list<std::pair<std::string,
        std::list<std::string>>>::iterator paso;

        for (paso = it->second.begin(); paso != it->second.end(); ++paso) {
            bool para_cancelar = execute(paso->first, paso->second, false);
            if (!para_cancelar) {
                std::cout << "Macro " << name << " detenido por error en comando interno.\n";
                return;
            }
        }

        std::string historial_despues = entity.obtener_status_para_historial();

        std::string datos =
            "Comando: MACRO " + name +
            "\nANTES: " + historial_antes +
            "\nDESPUES: " + historial_despues + "\n";

        history.push_back(datos);
    }
};

//ComandReset
void comandoReset(const std::list<std::string>& args, Entity& e) {
    if (!args.empty()) {
        std::cout << "Error: reset no recibe argumentos.\n";
        return;
    }
    e.reset();
}

//Una clase HealComand para comando como functor
class HealCommand {
    private:
    Entity& jugador;
    int usos;

    public:
    HealCommand(Entity& e) : jugador(e), usos(0) {}

    void operator()(const std::list<std::string>& args)
    {
        if (args.size() != 1) {
            std::cout << "Error: healFunctor requiere 1 argumento.\n";
            return;
        }

        try {
            int valor = std::stoi(*args.begin());
            usos++;
            std::cout << "healFunctor usado " << usos << " veces.\n";
            jugador.heal(valor);
        }
        catch (...)
        {
            std::cout << "Error: healFunctor necesita numero valido.\n";
        }
    }
};

int main() {

    // Crear jugador y mostrar estado inicial
    Entity jugador("Humano", "hacha");

    std::cout << "\nValores actuales del jugador:\n";
    jugador.status();

    // Crear centro de comandos
    CommandCenter center(jugador);

    // ------------------------------------------------------------
    // Registro de comandos
    // ------------------------------------------------------------
    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << "REGISTRO DE COMANDOS\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    // Comando heal
    std::cout << "\nRegistrando heal\n";

    center.registerCommand("heal", [&](const std::list<std::string>& args) {

        if (args.size() != 1) {
            std::cout << "Error: heal requiere exactamente 1 argumento.\n";
            return;
        }

        try {
            int value = std::stoi(*args.begin());
            jugador.heal(value);
        }
        catch (...) {
            std::cout << "Error: heal necesita un numero valido.\n";
        }
    });

    // Comando status
    std::cout << "Registrando status\n";

    center.registerCommand("status", [&](const std::list<std::string>& args) {

        if (!args.empty()) {
            std::cout << "Error: status no recibe argumentos.\n";
            return;
        }

        jugador.status();
    });

    // Comando damage
    std::cout << "Registrando damage\n";

    center.registerCommand("damage", [&](const std::list<std::string>& args) {

        if (args.size() != 1) {
            std::cout << "Error: damage requiere 1 argumento.\n";
            return;
        }

        try {
            int valor = std::stoi(args.front());
            jugador.damage(valor);
        }
        catch (...) {
            std::cout << "Error: damage necesita un numero valido.\n";
        }
    });

    // Comando move
    std::cout << "Registrando move\n";

    center.registerCommand("move", [&](const std::list<std::string>& args) {

        if (args.size() != 2) {
            std::cout << "Error: move requiere exactamente 2 argumentos.\n";
            return;
        }

        try {
            std::list<std::string>::const_iterator it = args.begin();

            int x = std::stoi(*it);
            ++it;
            int y = std::stoi(*it);

            jugador.move(x, y);
        }
        catch (...) {
            std::cout << "Error: move necesita numeros validos.\n";
        }
    });

    // Comando addResource
    std::cout << "Registrando addResource\n";

    center.registerCommand("addResource", [&](const std::list<std::string>& args) {
        if (args.size() != 1) {
            std::cout << "Error: addResource requiere 1 argumento.\n";
            return;
        }
        jugador.addResource(args.front());
    });

    // Comando reset
    std::cout << "Registrando reset\n";

    center.registerCommand("reset", [&](const std::list<std::string>& args) {
        comandoReset(args, jugador);
    });

    // Comando con functor
    std::cout << "Registrando healFunctor\n";

    HealCommand healCmd(jugador);
    center.registerCommand("healFunctor", healCmd);

    // ------------------------------------------------------------
    // Pruebas de comandos
    // ------------------------------------------------------------
    std::cout << "\n-------------------------------------------------------------------------------\n";
    std::cout << "PRUEBAS DE COMANDOS\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas heal
    std::cout << "Implementacion de heal\n";

    center.execute("heal", {"10"});
    center.execute("status", {});

    center.execute("heal", {"abc"});
    center.execute("heal", {});

    center.execute("reset", {});

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas damage
    std::cout << "Implementacion de damage\n";

    center.execute("damage", {"20"});
    center.execute("status", {});

    center.execute("damage", {"xyz"});
    center.execute("damage", {});

    center.execute("reset", {});

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas move
    std::cout << "Implementacion de move\n";

    center.execute("move", {"5", "5"});
    center.execute("status", {});

    center.execute("move", {"x", "15"});
    center.execute("move", {"10"});

    center.execute("reset", {});

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas addResource
    std::cout << "Implementacion de addResource\n";

    center.execute("addResource", {"oro"});
    center.execute("status", {});

    center.execute("addResource", {"espada"});
    center.execute("status", {});

    center.execute("addResource", {});

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas status
    std::cout << "Implementacion de status\n";

    center.execute("status", {});
    center.execute("status", {"1"});
    center.execute("status", {"extra"});

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas removeCommand
    std::cout << "Implementacion de removeCommand\n";

    center.removeCommand("comandoFake");
    center.removeCommand("heal");

    center.execute("heal", {"10"});

    center.removeCommand("heal");

    std::cout << "-------------------------------------------------------------------------------\n";

    // Pruebas healFunctor
    std::cout << "Implementacion de healFunctor\n\n";

    center.execute("healFunctor", {"25"});
    center.execute("status", {});

    center.execute("healFunctor", {"0"});
    center.execute("status", {});

    center.execute("healFunctor", {"TIMANA"});
    center.execute("healFunctor", {});

    center.execute("reset", {});

    // ------------------------------------------------------------
    // Macros
    // ------------------------------------------------------------
    std::cout << "-------------------------------------------------------------------------------\n";
    std::cout << "COMANDOS COMPUESTOS\n";
    std::cout << "-------------------------------------------------------------------------------\n";

    // Macros heal
    std::cout << "\nImplementacion de full_heal\n";

    std::list<std::pair<std::string, std::list<std::string>>> macro1heal = {
        {"healFunctor", {"50"}},
        {"status", {}}
    };

    center.registerMacro("full_heal1", macro1heal);
    center.executeMacro("full_heal1");

    std::list<std::pair<std::string, std::list<std::string>>> macro2heal = {
        {"healFunctor", {"0"}},
        {"status", {}}
    };

    center.registerMacro("full_heal2", macro2heal);
    center.executeMacro("full_heal2");

    std::list<std::pair<std::string, std::list<std::string>>> macro3heal = {
        {"healFunctor", {}},
        {"status", {}}
    };

    center.registerMacro("full_heal3", macro3heal);
    center.executeMacro("full_heal3");

    std::cout << "-------------------------------------------------------------------------------\n";

    // Macros damage
    std::cout << "\nImplementacion de full_damage\n";

    std::list<std::pair<std::string, std::list<std::string>>> macro1damage = {
        {"damage", {"20"}},
        {"status", {}}
    };

    center.registerMacro("full_damage1", macro1damage);
    center.executeMacro("full_damage1");

    std::list<std::pair<std::string, std::list<std::string>>> macro2damage = {
        {"damage", {"0"}},
        {"status", {}}
    };

    center.registerMacro("full_damage2", macro2damage);
    center.executeMacro("full_damage2");

    std::list<std::pair<std::string, std::list<std::string>>> macro3damage = {
        {"damage", {}},
        {"status", {}}
    };

    center.registerMacro("full_damage3", macro3damage);
    center.executeMacro("full_damage3");

    std::cout << "-------------------------------------------------------------------------------\n";

    // Macros move
    std::cout << "\nImplementacion de full_move\n";

    std::list<std::pair<std::string, std::list<std::string>>> macro1move = {
        {"move", {"5", "5"}},
        {"status", {}}
    };

    center.registerMacro("full_move1", macro1move);
    center.executeMacro("full_move1");

    std::list<std::pair<std::string, std::list<std::string>>> macro2move = {
        {"move", {"0", "0"}},
        {"status", {}}
    };

    center.registerMacro("full_move2", macro2move);
    center.executeMacro("full_move2");

    std::list<std::pair<std::string, std::list<std::string>>> macro3move = {
        {"move", {"10"}},
        {"status", {}}
    };

    center.registerMacro("full_move3", macro3move);
    center.executeMacro("full_move3");

    std::cout << "-------------------------------------------------------------------------------\n";

    // Macro con error interno
    std::cout << "\nDetener macro si falla un comando\n";

    std::list<std::pair<std::string, std::list<std::string>>> macro_falla = {
        {"comando_inexistente", {"5", "5"}},
        {"status", {}}
    };

    center.registerMacro("macro_falla", macro_falla);
    center.executeMacro("macro_falla");

    // ------------------------------------------------------------
    // Historial final
    // ------------------------------------------------------------
    std::cout << "\n-------------------------------------------------------------------------------\n";
    std::cout << "HISTORIAL COMPLETO\n";
    std::cout << "-------------------------------------------------------------------------------\n\n";

    center.mostrarhistorial();
    return 0;
}