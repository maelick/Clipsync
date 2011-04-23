#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>
#include <iostream>

using namespace Poco::Crypto;
using namespace std;

int main()
{
  CipherFactory &cf = CipherFactory::defaultFactory();
  string passphrase = "coucou";
  string salt("asdff8723lasdf(**923412");
  CipherKey ck("aes-256-cbc", passphrase, salt);
  Cipher *c = cf.createCipher(ck);

  string plain = "Coucou c'est moi";
  string encrypted = c->encryptString(plain);
  string decrypted = c->decryptString(encrypted);

  cout << plain << endl;
  cout << encrypted << endl;
  cout << decrypted << endl;
}
