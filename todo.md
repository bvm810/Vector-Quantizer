### Lista de Tarefas Quantizador

* Ler imagem com formato informado no cabeçalho em uma matriz **OK**
* Dividir a matriz em blocos N x M **OK**
* Considerar cada bloco como um vetor de tamanho NM e realizar K-Means **OK**
* Testar diferentes blocos para ver o MSE
* Testar diferentes Ks para ver o MSE
* Mandar usando codificador aritmético
  * As dimensões dos blocos
  * A tabela com as NM coordenadas dos vetores
  * As chaves de cada um dos vetores
* Decodificar usando codificador aritmético
* Recriar blocos a partir dos centroides **OK**
* Reconstruir a matriz a partir dos blocos **OK**
* Escrever a imagem **OK**