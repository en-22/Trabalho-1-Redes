GRR20221226 - Eloisa Nielsen
GRR20221259 - Agnaldo dos Santos Prieto Junior
GRR20221241 - Guilherme Stonoga Tedardi

Bibliotecas:

funcClient - Funções relacionadas ao cliente.
  checkDiskUsage
  recieveDescription
  recieveVideo
  recieveNames
  showDownloadMenu
  showMenu

funcServer - Funções relacionadas ao servidor.
  sendVidList
  sendVideo
  sendDescription
  videoExists

utils - Funções basicas para envio de frames, verificação de pacotes, entre outros.
  sendError
  crc8
  mountPacket
  sendACK
  sendNACK
  sendPacket
  recievePacket
  waitforACK

Os videos disponiveis devem ser salvos no diretorio 'videos'.

Se o servidor não estiver em execução, todas as chamadas do cliente resultarão em timeout.
