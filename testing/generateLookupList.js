

// ; , / ? : @ & = + $ - _ . ! ~ * ' ( ) #

// first line write number of character
var charprinted = "";
for(var i =0;i<= 255; ++i) {
    charprinted += ",";
    var strCount = "" + i;
    for(var j = strCount.length; j<= 3; ++j) { charprinted += " "; }
    charprinted += "" + i;
}
console.log(charprinted);





charprinted = "{ ";
for(var i =0;i<= 255; ++i) {
    if(i < 33 || i >126) { 
        charprinted += ",   0";
        continue;
    }
    charprinted += ", '" + String.fromCharCode(i) + "'";
}
charprinted += "}";
console.log(charprinted);