Object Pooling
As I shared in my first post, I started programming games without much ambition — I just wanted to have fun with my friends. Back then, I didn’t really think about what was happening behind the scenes, especially when it came to memory management or performance. I just wanted to make little games, and that was it!

However, as I dove deeper, studying programming more seriously and working professionally, I learned that creating and destroying objects all the time comes at a cost. And this becomes crucial when porting games to consoles or when we need to ensure a game runs well on more limited systems.

In any case, in real-time applications like games — and even more so in embedded systems — efficient memory management is ESSENTIAL. That’s when I discovered the concept of Object Pooling. I remember that some veteran friends had mentioned it before, but I completely ignored it at the time.

What is Object Pooling?
Instead of creating and destroying objects during gameplay (which consumes processing power and constantly allocates/deallocates memory), Object Pooling is a technique where you pre-create a set of objects and reuse them.

When you need an object, you take an available one from the pool.
When you're done using it, you reset it and return it to the pool for future use.

This avoids costly operations like memory allocation and garbage collection, making your game much faster and more stable — especially in situations with lots of projectiles, enemies, effects, or particles.

Notice in the GIF below that our friend Megaman X can only fire up to three bullets at a time. This is a classic example of object pooling: each projectile is kept "off-screen" in a pool of three, deactivated and waiting for the player to fire.

![GIF]

Why is Object Pooling so important?
Performance Boost: Reduces CPU and memory usage.

Consistency: Prevents random frame drops or micro-freezes.

Control: You know exactly how many objects exist at any moment.

Scalability: Helps your game handle large numbers of simultaneous objects better.

Even with today’s powerful consoles, we can’t go overboard with objects being constantly created and destroyed. And in embedded systems, where resources are extremely limited, techniques like object pooling aren't just good practices — they’re necessary.

Is it really necessary to implement Object Pooling?
A common question — especially for those using modern engines like Unity, Godot, or Unreal — is:
"If there’s already automatic Garbage Collection, do I still need to worry about this?"

I’ve asked myself that same question before.

The answer is: it depends on your project.

For small games or scenes that don’t frequently instantiate/destroy objects, the engine’s garbage collector can handle everything without noticeable issues.

However, in games that involve a high volume of rapid instances (e.g., shooters, hack-and-slash, bullet hells, or intense particle systems), constant creation and destruction will cause overhead — and at that point, even the garbage collector won’t prevent performance drops.

In those cases, implementing Object Pooling manually can make a huge difference in keeping your game smooth and responsive.

Simple Example of Object Pooling
Imagine a simple projectile pool:

At the start of the game, you create 100 projectiles and keep them deactivated.

When the player shoots, you activate an available projectile from the pool instead of creating a new one.
When the projectile hits something or leaves the screen, you deactivate it and return it to the pool.

Here's a basic pseudocode example:

pseudo
Copiar
Editar
pool = createProjectilePool(size=100)

function shoot():
    projectile = pool.getAvailableProjectile()
    if projectile != null:
        projectile.activate(position, direction)

function onProjectileDestroyed(projectile):
    projectile.deactivate()
    pool.returnProjectile(projectile)
I initially started writing an example in C, but it was getting way too big, so I decided to simplify it with this small block of pseudocode instead.

As you can see, the shooting function fetches a reference to an available projectile (with an inactive status) and activates it, repositioning it accordingly.

When a projectile is destroyed, we simply change its status flag back to DEACTIVATED.
A deactivated projectile isn't rendered or processed, saving a lot of resources.

Final Thoughts
The truth is, many modern game development tools already have very efficient garbage collection mechanisms, which often makes object pooling seem obscure or unnecessary.

However, when developing games for limited systems — like a small game running on a microcontroller — every byte of memory matters, and object pooling becomes absolutely essential.
Not to mention that it’s a way to build faster, more stable, and more optimized games overall.

Of course, if you're just starting your journey in game development, don’t worry if these optimizations seem complex right now. With time and practice, they'll become second nature.

And if you've already mastered the basics, learning and implementing techniques like Object Pooling is an incredible next step to take your projects to the next level!

Thanks for reading, and I hope this little knowledge drop was helpful to you!

#GameDev #Programming #Optimization #GameDevelopment #MemoryManagement

Algumas sugestões que podem deixar o post ainda mais legal:
Talvez você queira adicionar um GIF ou imagem simples para ilustrar visualmente como a pool funciona (tipo mostrando uma caixa cheia de projéteis inativos).

Você poderia listar cenários comuns onde object pooling é quase obrigatório, como "sistemas de partículas" ou "spawns massivos de inimigos".

Pode adicionar uma menção a "design patterns", já que o Object Pool é um dos padrões de design mais conhecidos (relacionado ao livro Gang of Four).

E talvez, se quiser aprofundar, pode citar que existem variações de Pooling, como Pools Dinâmicos (que aumentam se necessário) ou Pools por Tipo.