#include <iostream>
#include <fstream>
#include <vector>
#include "dct2.h"

using namespace std;


float** read_raw_img(string file_name) {
    ifstream rawFile(file_name, ios::in | ios::binary);

    // 獲取檔案大小
    rawFile.seekg(0, ios::end);
    streampos fileSize = rawFile.tellg();
    rawFile.seekg(0, ios::beg);

    // 讀取所有數據到vector
    vector<unsigned char> temp_vec(fileSize);
    rawFile.read(reinterpret_cast<char*>(temp_vec.data()), fileSize);


    // 二维 float array
    int n = sqrt(int(fileSize));
    float** original_img = new float* [n]; // 创建指向 float 指针的指针

    for (int i = 0; i < n; ++i) {
        original_img[i] = new float[n]; // 为每行创建 float 数组
    }

    // 讀取 vector 數據到 float array
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            original_img[i][j] = temp_vec[i*n+j];
        }
    }

    cout << "已成功讀取 " << int(fileSize) << " 個字節的數據" << endl << endl;

    // 關閉檔案
    rawFile.close();

    return original_img;
}

void quantize(float** x, int QF) {
    int QUAN_MATRIX[8][8] = {
        {16, 11, 10, 16, 24, 40, 51, 61},
        {12, 12, 14, 19, 26, 58, 60, 55},
        {14, 13, 16, 24, 40, 57, 69, 56},
        {14, 17, 22, 29, 51, 87, 80, 62},
        {18, 22, 37, 56, 68, 109, 103, 77},
        {24, 35, 55, 64, 81, 104, 113, 92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103, 99},
    };
    float factor;
    if (QF < 50){
        factor = 5000 / QF;
    }
    else {
        factor = 200 - 2 * QF;
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float qij = QUAN_MATRIX[i][j] * factor / 100;
            x[i][j] = x[i][j] / qij;
        }
    }
}

int main() {
    int choice;
    string img_name;
    string process_type;
    string data_path = "./Test Images/";

    cout << "Lena: 1" << endl;
    cout << "Baboon: 2" << endl;
    cout << "Please choose the image: ";
    //cin >> choice;
    cout << endl;
    choice = 1;

    if (choice == 1) {
        img_name = "Lena";
    }
    else if (choice == 2) {
        img_name = "Baboon";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    cout << "Gray: 1" << endl;
    cout << "RGB: 2" << endl;
    cout << "Please choose the color type: ";
    //cin >> choice;
    cout << endl;
    choice = 1;

    if (choice == 1) {
        process_type = "";
        data_path += "GrayImages/";
    }
    else if (choice == 2) {
        process_type = "RGB";
        data_path += "ColorImages/";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    //根據使用者選擇的檔案名稱和處理方式執行相應的處理
    string raw_name = data_path + img_name + process_type + ".raw"; //"./Data/RAW/lena_decode.raw";
    cout << raw_name << " Loading..." << endl;

    float **original_img = read_raw_img(raw_name);
    int n = 8;
    cout << original_img[255][256] << endl;
    cout << original_img[256][256] << endl;

    // scan 512 * 512 with 8 * 8 DCT
    int x_pos = 0, y_pos = 0;
    while (y_pos < 512) {
        // create empty 8*8 block
        float** block = new float* [8];
        for (int i = 0; i < 8; ++i) {
            block[i] = new float[8]; // 为每行创建 float 数组
        }
        // scan 8*8 block from 512*512 image
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                block[i][j] = original_img[x_pos + i][y_pos + j];
            }
        }
        // dct processing
        dct2(block, n);
        // push 8*8 block back to original image
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                original_img[x_pos + i][y_pos + j] = block[i][j];
            }
        }
        // move to next 8*8 position
        x_pos += 8;
        if (x_pos == 512) {
            y_pos += 8;
            x_pos = 0;
        }
    }
    
    cout << original_img[255][256] << endl;
    cout << original_img[256][256] << endl;

    // Quantization
    const int QF = 50;
    quantize(original_img, QF);


	return 0;
}