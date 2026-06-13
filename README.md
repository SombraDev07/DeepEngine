# Engine

Game engine moderna baseada em sistemas comprovados da CRYENGINE, reconstruídos com stack moderna (C++20, Vulkan, EnTT ECS, Jolt Physics).

## Stack

| Camada | Tecnologia | Licença |
|--------|-----------|---------|
| Linguagem | C++20 | - |
| Build | CMake 3.20+ | BSD |
| Render | bgfx → Vulkan/DX12/Metal | BSD |
| ECS | EnTT | MIT |
| Física | Jolt Physics | MIT |
| Áudio | MiniAudio | MIT |
| UI Editor | Dear ImGui | MIT |
| Ferramentas | Python 3 | PSF |

## Estrutura

```
core/       Sistemas de baixo nível (math, memory, jobs, platform)
physics/    Física (Jolt Physics)
render/     Renderização + SVO GI
animation/  Animation State Machine + IK + Blend Spaces
editor/     Editor visual (NodeGraph framework)
tools/      Asset pipeline, scripts
thirdparty/ Dependências externas
```

## Roadmap

Ver `.roadmap` para o plano completo.
