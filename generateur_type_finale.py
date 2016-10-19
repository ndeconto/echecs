# -*- coding: cp1252 -*-
def generer_type_finale(nb_piece):
    """
        renvoie tous les types de finales possibles a nb_piece pieces
        sous forme d'une liste de chaines de caracteres
        par exemple ["RRD", "RRT", "RRC", "RRF", "RRP"] pour les 3 pieces
    """

    fct_tri = lambda x : {"R" : 0, "D" : 1, "T" : 2, "F" : 3, "C" : 4, "P" : 5}[x]

    def aux(n):

        if n <= 2:
            return set(["RR"])

        new = set()

        for f in aux(n - 1):
            for p in "DTFCP": new.add("".join(sorted(f + p, key = fct_tri)))
        return new

    return [f for f in aux(nb_piece)]


for i in range(3, 7):
    with open("type_finale/" + str(i) + "pieces.txt", "w") as f:
        f.write("\n".join(generer_type_finale(i)))
            
        
