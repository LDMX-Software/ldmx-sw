import copy
import types
import warnings
from dataclasses import Field, dataclass, field
from typing import Any


def check_list(attr, the_list, dimension, entry_type):
    """make sure each entry in a list is the right type"""
    if dimension == 1:
        if entry_type is not Any:
            for e in the_list:
                if not isinstance(e, entry_type):
                    raise TypeError(
                        f"Entry {e} in parameter {attr} "
                        "is not of expected type {entry_type}"
                    )
    elif dimension == 2:
        for sub_list in the_list:
            check_list(attr, sub_list, 1, entry_type)
    else:
        raise Exception(f"List with dimension {dimension} not supported.")


def validate_and_set_attr(self, attr, val):
    """make sure the passed value is valid for the attribute it s updating

    This function becomes the __setattr__ function for any parameter_set
    class and is the core logic that ensures parameters are not mis-spelled
    and do not change type.
    We ensure this type-solidity by using dataclass to process a class's
    definition for member annotations (see parameter_set). This produces
    an internal list called __dataclass_fields__ that contains our set
    of attributes (and their types!).

    Besides this type checking, we can also remap if the user defines a
    class variable __legacy__ that maps old parameter names to new ones.

    Parameter
    ---------
    self: object
        object whose attributes we are validating
    attr: str
        name of attribute we are attempting to set
    val: Any
        value we want to set the attribute to

    Raises
    ------
    KeyError:
        if the passed attribute is not one of the members as defined
        when the class was declared and parsed by dataclass
    TypeError:
        if the passed value does not match the type of the member
        as defined when the class was declared and parsed by dataclass

    Warnings
    --------
    If the legacy re-mapping of a name is applied, a DeprecationWarning
    is issued.
    """

    # first, we check if the attr is a legacy name
    legacy_remap = getattr(self.__class__, "__legacy__", None)
    if legacy_remap is not None and attr in legacy_remap:
        new_attr = legacy_remap[attr]
        warnings.warn(
            f"Legacy name '{attr}' has been replaced by '{new_attr}'.",
            DeprecationWarning,
            stacklevel=2,
            # stacklevel=2
        )

        # the attr is now updated to new name
        attr = new_attr
        member, _dot, submember = attr.partition(".")
        if submember:
            # we are reaching into a sub-class and so we recurse into it
            if member not in self.__dataclass_fields__:
                raise KeyError(
                    f"Attribute {member} not a member of {self.__class__.__name__}"
                )
            # validate intermediate type?
            # I can't think of a reason to do this because we only reach
            # here in the case where a developer is remapping a legacy
            # parameter into a new submember parameter

            return validate_and_set_attr(self.__dict__[member], submember, val)
        # no submember, just fall through after we have changed 'attr'

    if attr not in self.__dataclass_fields__:
        raise KeyError(f"Attribute {attr} not a member of {self.__class__.__name__}")

    expected_type = self.__dataclass_fields__[attr].type
    if isinstance(val, list):
        expected_dimension = self.__dataclass_fields__[attr].metadata["dimension"]
        expected_entry_type = self.__dataclass_fields__[attr].metadata["entry_type"]
        check_list(attr, val, expected_dimension, expected_entry_type)
    elif expected_type is not Any and not isinstance(val, getattr(expected_type, "__origin__", expected_type)):
        raise TypeError(
            f"Attribute {attr} should be type {expected_type} "
            f"instead of type {type(val)}."
        )

    self.__dict__[attr] = val


class CreateDefault:
    """stand in factory for a dataclass field when we already have a default

    dataclasses do not want mutable classes provided as default values,
    instead we need to provide a default_factory to the dataclass field.
    This class can hold an already created default value of any type and then
    return a **full** copy of that default value for the destination object
    to edit it as it wishes.
    """

    def __init__(self, o):
        self._default = o

    def __call__(self):
        return copy.deepcopy(self._default)


def parameter_set(
    _class=None,
    *,
    post_init=None,
    helpers=None,
    required_base=None,
    **required_parameters,
):
    """decorate a class and have it become a parameter_set whose members cannot change

    This function decorates a class when it is defined, allowing us to modify
    the class's behavior before it is used. We do a few things:
    - add (or update) the __post_init__ function to ensure that all dataclass fields
      are present in the __dict__ member variable (and include the passed post_init
      if provided)
    - have the class use valid_and_set_attr as the __setattr__ function
    - add the list of required_parameters to the type annotations and class values
      (if provided)
    - add the passed list of helper functions (if provided)
    - check that the class has the required_base class (if provided)
    - wrap values provided by the user in CreateDefault if they are mutable
    - forward the updated class to dataclass for final processing

    See Also
    --------
    validate_and_set_attr:
        for how the actual validation is done at run time
    dataclasses.dataclass:
        for general background on dataclasses and their vocabulary

    Parameters
    ----------
    _class:
        The class being decorated
    post_init: Callable
        Additional function to *include* in the generated __post_init__ function
        This is run _after_ the dataclass fields are copied into the __dict__,
        but before the class's own __post_init__ if it is defined.
    helpers: List[Tuple[str,Callable]]
        Additional functions to assign as member functions to the class
    required_base: type
        Required base type that classes this function is decorating should be
    required_parameters: Dict[str,Any]
        Additional key-word parameters are included as additional parameters
        in the parameter_set by injecting their annotations and values into
        the class definition.

    Examples
    --------
    The most simple case is where we have a Python class that we want to hold
    a few parameters in a group so that the C++ can retrieve them later.

        @parameter_set
        class MyParameters:
            my_integer: int
            my_double: float = 42.0

    In this case, `my_integer` is "required" in the sense that it must be provided
    whenever constructing a `MyParameters` object. This is helpful for parameters
    that the user must provide and don't have a sensible default. `my_double`, on
    the other hand, is "optional" and has the default value of 42.0.

    The more complicated but more common case is where many different classes on the
    C++ side must have the same few parameters to distinguish themselves but then can
    have different, additional parameters. (Think event processors, conditions object
    providers, primary generators, user actions, ...). The way to handle this situation
    is to wrap the parameter_set function by another function that _requires_ certain
    arguments. This then forces the user to provide certain values for certain
    parameters when they are constructing a new Python configuration class.
    The parameters to this function are mainly focused on this use case - examples
    of which you can see in _processor.py and _conditions_object_provider.py.

    Warnings
    --------
    Groups of parameter sets that have similar types on the C++ side tend to also
    be grouped on the Python side using a common base class to help do type checking.
    However, we *cannot* use all the features of Python inheritance with the current
    solution. Specifically,
    1. The base class cannot define any shared parameters since then those parameters
        are included before any of the derived classes parameters. Pass those
        to this function.
    2. The base class should not define a __post_init__ or __setattr__ function
        both of which will be overwritten by the versions of those functions defined
        here.
    """

    if helpers is None:
        helpers = []

    def _decorator_impl(cls):
        if required_base is not None and required_base not in cls.__bases__:
            raise TypeError(f"{cls.__name__} is required to have base {required_base}")

        passed_post_init = post_init if post_init is not None else lambda self: None

        orig_post_init = (
            cls.__post_init__ if hasattr(cls, "__post_init__") else lambda self: None
        )

        # update post_init function to include some kind of common tasks
        def _full_post_init(self):
            # make sure all dataclass fields are present in __dict__
            # since that is the mechanism we use to grab them in C++
            for name, _field in self.__dataclass_fields__.items():
                if name not in self.__dict__:
                    self.__dict__[name] = getattr(self, name, None)
            passed_post_init(self)
            orig_post_init(self)

        cls.__post_init__ = _full_post_init

        # update setattr function to validate stuff as well
        cls.__setattr__ = validate_and_set_attr

        # add additional annotations that are required parameters
        for name, value in required_parameters.items():
            if type(value) is list:
                cls.__annotations__[name] = list[Any]
            elif type(value) is Field:
                cls.__annotations__[name] = type(value.default)
            else:
                cls.__annotations__[name] = type(value)
            setattr(cls, name, value)

        # add additional helpers
        for name, func in helpers:
            setattr(cls, name, func)

        # process class annotations so that mutable classes
        # are given to dataclass as a default_factory instead of a default
        for name, the_type in cls.__annotations__.items():
            the_value = getattr(cls, name, None)
            simple_types = (bool, int, float, str)

            if the_type in simple_types:
                # type is simple and can be left alone for dataclass to handle
                continue

            if isinstance(the_value, Field):
                # value already specified as dataclass field,
                # assume user knows what they are doing and skip
                continue

            if isinstance(the_type, types.GenericAlias) and the_type.__origin__ is list:
                # special list deduction to do early warnings on bad dimensions
                # and check entries in default value
                type_args = getattr(the_type, "__args__", ())
                dimension = len(type_args)
                if dimension == 0:
                    raise TypeError(
                        "Python configuration parameter_sets need to "
                        "annotate some content for each list."
                    )
                if dimension > 2:
                    raise TypeError(
                        "Python configuration parameter_sets are limited "
                        "to 1D or 2D lists."
                    )

                entry_type = type_args[0]
                if not all(entry_type is t for t in type_args):
                    raise TypeError(
                        "Python configuration parameter_sets must have all "
                        "entries be the same type."
                    )

                field_kwargs = {
                    "metadata": {"entry_type": entry_type, "dimension": dimension}
                }
                if the_value is not None:
                    check_list(name, the_value, dimension, entry_type)
                    field_kwargs["default_factory"] = CreateDefault(the_value)
                setattr(cls, name, field(**field_kwargs))
            else:
                # type is not simple, not already a dataclasses.Field, and not a list
                # so we wrap it in a creation function to get around dataclass's
                # prevention on using mutable defaults
                setattr(cls, name, field(default_factory=CreateDefault(the_value)))

        return dataclass(slots=False)(cls)

    if _class is None:
        return _decorator_impl
    else:
        return _decorator_impl(_class)
