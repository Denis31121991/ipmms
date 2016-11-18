# ipmms
  Программа для обнаружения объектов на изображениях, полученных при круговом микросканировании
  
  Основная задача программы – обнаружения объектов известной формы на изображениях с коррелированными шумами.
  
  Программа реализует следующие функции:
Оценка ковариационной матрицы шума по входному изображению и сохранение ее в файл для последующего использования.
Нанесение на изображение объекта, форма которого задается во входном файле или параметрически.
Расчет оптимального линейного фильтра по заданной форме объекта и ковариационной матрице шума. 
Фильтрация и обнаружение объекта с использованием полученного фильтра, расчет и вывод статистических параметров обработки. 
