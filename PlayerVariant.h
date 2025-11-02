#ifndef PLAYER_VARIANT_H
#define PLAYER_VARIANT_H

enum class PlayerVariant {
    PADRAO = 0,
    VERMELHO = 1,
    VERDE = 2,
    ROXO = 3
};

extern PlayerVariant g_selectedVariant; // definido em Jogo.cpp

#endif // PLAYER_VARIANT_H
