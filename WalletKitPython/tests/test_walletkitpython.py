import unittest
from walletkit import Hasher, HasherType


class TestHasher(unittest.TestCase):
    def test_do_hash(self):
        h = Hasher(HasherType.SHA256)
        v = h.hash('hello'.encode('utf8'))
        self.assertEqual(v.hex(), '2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824')
