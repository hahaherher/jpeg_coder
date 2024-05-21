# Jpeg Coder

## 環境
我是用 `visual studio c++ 2022` 打開新專案，把檔案都放在跟.sln檔案同個路徑

## 使用方法

1. 記得要先放Test Images(老師提供的圖片)，放在跟.sln同個路徑底下, 圖片路徑會像是 `./Test Images/GrayImages/Lena.raw`
2. 直接執行 `jpeg_coder.py`
3. 根據cmd的指示，輸入lena或baboon的代號
4. 接著，請輸入gray或RGB的代號，如果是RGB，還要輸入subsampling mode
5. cmd 上面會顯示PSNR的比較，還有檔案大小等等
6. `.hahajpg` 檔會出現在跟 `.sln` 同個路徑上
7. 解回來的 `.raw` 也在一樣的路徑上，可以用 `readraw.py` 把 `.raw` 轉成 `.jpg `來看