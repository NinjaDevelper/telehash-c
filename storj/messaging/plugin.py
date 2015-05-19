import importlib
import pkgutil

from storj.messaging.messaging import Messaging


def get_messaging_classes():
    """
    get class which implements Messaging methods.
    """
    count = 0
    cls = []
    import storj.messaging
    for (importer, name, isPkg) in pkgutil.iter_modules(
                                    path=storj.messaging.__path__):
        if isPkg:
            m = importlib.import_module('storj.messaging.' + name)
            if hasattr(m, 'get_messaging_class'):
                method = getattr(m, 'get_messaging_class')
                c = method()
                if issubclass(c, Messaging):
                    cls.append(c)
    return cls
