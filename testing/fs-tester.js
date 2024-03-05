/*
 * TS, 2020-2024
 * This is a NodeJS bases test script doing various filesystem tests
 *
 */

const fs = require('fs');
const async = require('async');
const basePath = "../build/testdir"

const files = ["/Agreatconstruction.svg","/Super Shapered.svg","/AnotherFolder/Another Construction as  well.svg", "/AnotherFolder/Shaper Logo.svg"];
var filesIndex = 0;
var iterations = 0;



async function testOpenClose() {
//    ++threadIndex;
    
    while(true) {
        ++filesIndex;
        if(filesIndex >= files.length) filesIndex = 0;
        
        var fileToLoad = files[filesIndex];

        console.log("In " + iterations + " working on: " + fileToLoad);

        try {
            var fileHandle = fs.openSync(basePath + fileToLoad, 'r');
            await new Promise(r => setTimeout(r, 200));
            fs.closeSync(fileHandle); 
        } catch(e) {
            console.error("error opening/closing file: ", e);
            process.exit(1);
        }

        var randomWait = Math.random() * (100 - 10) + 10;
        await new Promise(r => setTimeout(r, randomWait));


        ++iterations;
    }
}
testOpenClose();
