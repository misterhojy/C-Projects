#include "bacon.h"
#include <string.h>
#include <ctype.h>
// Add other #includes here if you want.

int encrypt(const char *plaintext, char *ciphertext) {
    unsigned int i, count, letters, groups, bits, bacon, whilecount, totalbits;
    int cindex = 0;
    letters = 0;
    //count all letters in text
    for(i = 0; i < strlen(ciphertext); i++){    
        if((ciphertext[i] >= 'A' && ciphertext[i] <= 'Z') || (ciphertext[i] >= 'a' && ciphertext[i] <= 'z')) {
            ++letters;
        } 
    }

    //how many letters we can code
    groups = (letters/6); 
    //immediately return when edge case of having not even EOM  
    if(letters < 6){    
        return - 1;
    //when groups from cipher longer than str plaintext length add EOM
    } else if (groups > strlen(plaintext)){  
        groups = strlen(plaintext) + 1;
    }                      

    totalbits = groups * 6;
    count = 0;
    //iterating at each encrypted letter
    for(i = 0; i < (groups - 1); i++){ 
        //resetting bits and while count to 0 at each iteration   
        bits = 0;   
        whilecount = 0;
        //finding each bacon code
        char current = plaintext[i];
        if (current >= 'A' && current <= 'Z'){
            bacon = current - 'A';
        } else if (current >= 'a' && current <= 'z'){
            bacon = current - 'a';
        } else if (current >= ' ' && current <= ')'){
            bacon = current - 6;
        } else if (current >= ',' && current <= '/'){
            bacon = current - 8;
        } else if (current == 63){
            bacon = current - 11;
        }
        //adding to bacon to bits
        bits |= bacon; 
        //shift msb at most left 
        bits <<= (32-6);
        //while we encrypt 6 bits of the bacon code at a time and the amount of letters seen is less than amount of totalbits - 6 and cindex is less than strlen
        while((whilecount < 6) && (count < (totalbits - 6)) && (cindex < (int)strlen(ciphertext))){ 
            //coding cipher text one bacon at a time, reset bacon and do again
            //if it capital and msb 0 make lower and msb 1 make upper
            if(ciphertext[cindex] >= 'A' && ciphertext[cindex] <= 'Z'){   
                if((bits & 0x80000000) >> 31){  
                    ciphertext[cindex] = toupper(ciphertext[cindex]);
                } else {
                    ciphertext[cindex] = tolower(ciphertext[cindex]);
                }
                count++;
                whilecount++;
                bits<<=1;
            //if it lower and msb 0 make lower and msb 1 make upper
            } else if(ciphertext[cindex] >= 'a' && ciphertext[cindex] <= 'z'){   
                if((bits & 0x80000000) >> 31){
                    ciphertext[cindex] = toupper(ciphertext[cindex]);
                } else{
                    ciphertext[cindex] = tolower(ciphertext[cindex]);
                }
                count++;
                whilecount++;
                bits<<=1;
            }
            cindex++;
        }
    } 

    //making bits = the eom, add to the end or if its an empty string we add to the beginning
    bits |= 63; 
    //make it msb all left
    bits <<= (32-6); 
    //for loop that encountered letters is <= totalbits needed for encryption. iterating through ciphertext starting at either 0 or end of while till less than strlen
    for(i = cindex; (count < totalbits) && i < (strlen(ciphertext)); i++){
        if(ciphertext[i] >= 'A' && ciphertext[i] <= 'Z'){
            if((bits & 0x80000000) >> 31){  
                ciphertext[i] = toupper(ciphertext[i]);
            } else {
                ciphertext[i] = tolower(ciphertext[i]);
            }
            count++;
            bits<<=1;
        } else if(ciphertext[i] >= 'a' && ciphertext[i] <= 'z'){
            if((bits & 0x80000000) >> 31){
                ciphertext[i] = toupper(ciphertext[i]);
            } else{
                ciphertext[i] = tolower(ciphertext[i]);
            }
            count++;
            bits<<=1;
        }
    }
    return (groups - 1);
}


int decrypt(const char *ciphertext, char *plaintext) {
    unsigned int i, letters, groups, count, whilecount, bits, hasEOM, hasInvalid;
    int cindex;
    whilecount = 0;
    letters = 0;
    bits = 0;
    hasEOM = 0;
    hasInvalid = 0;
    //counting how many letters is in ciphertext
    for(i = 0; i < strlen(ciphertext);){  
        bits = 0;
        while((whilecount < 6)){
            if((ciphertext[i] >= 'A') && (ciphertext[i] <= 'Z')){
                bits<<=1;
                bits |= 1;
                letters++;
                whilecount++;
            } else if ((ciphertext[i] >= 'a') && (ciphertext[i] <= 'z')){
                bits<<=1;
                letters++;
                whilecount++;
            }
            i++; 
        }
        whilecount = 0;  
        if(bits == 63){
            hasEOM = 1;
        } else if((bits >= 53) && (bits <= 62)){   //invalid bacon code then return -3
            hasInvalid = 1;
        } 
    }
    //edge cases
    if (letters < 6){ 
        return -1;
    } else if(hasEOM != 1){
        return -2;
    } else if (hasInvalid) {
        return -3;
    }

    //setting all counts and indexes to zero
    count = 0;
    whilecount = 0;
    cindex = 0;
    groups = 0;
    bits = 0;
    hasEOM = 0;
    //loop for each element of plain text
    for(i = 0; i < strlen(plaintext); i++){
        bits = 0;
        //while count to get the bits value
        while((whilecount < 6)){
            if((ciphertext[cindex] >= 'A') && (ciphertext[cindex] <= 'Z')){
                bits<<=1;
                bits |= 1;
                count++;
                whilecount++;
            } else if ((ciphertext[cindex] >= 'a') && (ciphertext[cindex] <= 'z')){
                bits<<=1;
                count++;
                whilecount++;
            } 
            cindex++;
        }
        whilecount = 0;
        if (bits == 63){    //adding EOM
            hasEOM = 1;
            plaintext[i] = '\0';
            break;
        }
        //converting bacon code to normal char
        if(bits <= 25) {
            bits += 65;
        } else if ((bits >= 26) && (bits <= 35)) {
            bits += 6;
        } else if ((bits >= 36) && (bits <= 51)){
            bits += 8;
        } else if ((bits == 52)){
            bits += 11;
        }
        plaintext[i] = (char) bits;
        groups++;
    }
     return groups;
}
