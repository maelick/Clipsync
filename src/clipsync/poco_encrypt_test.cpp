/*
  Clipsync, clipboard synchronizer
  Copyright (C) 2011 Maëlick Claes (himself [at] maelick [dot] net)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
