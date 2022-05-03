from django.contrib.auth.forms import UserCreationForm
from django.contrib.auth.models import User
from django import forms
from django.forms import widgets
from django.forms.models import ModelForm

class RegisterUserForm(UserCreationForm):
    email = forms.EmailField(widget = forms.EmailInput(attrs={'class':'form-control'}))
    first_name = forms.CharField(widget = forms.TextInput(attrs={'class':'form-control'}))
    last_name = forms.CharField(widget = forms.TextInput(attrs={'class':'form-control'}))

    class Meta:
        model = User
        fields = ('username', 'first_name', "last_name",'email', 'password1', 'password2')

    def __init__(self, *args, **kwargs):
        super(RegisterUserForm, self).__init__(*args, **kwargs)

        self.fields['username'].widget.attrs['class'] = 'form-control'
        self.fields['password1'].widget.attrs['class'] = 'form-control'
        self.fields['password2'].widget.attrs['class'] = 'form-control'

class UpdateForm(ModelForm):
    class Meta:
        model = User
        fields = ('username', 'password', 'email', 'first_name', 'last_name')
        labels = {
            'username' : 'username',
            'password' : 'password',
            'email' : 'email',
            'first_name' : 'first_name',
            'last_name' : 'last_name',}
        widgets = {
            'username' : forms.TextInput(attrs={'class':'form-control'}),
            'password' : forms.TextInput(attrs={'class':'form-control'}),
            'email' : forms.EmailInput(attrs={'class':'form-control'}),
            'first_name' : forms.TextInput(attrs={'class':'form-control'}),
            'last_name' : forms.TextInput(attrs={'class':'form-control'}),
            }