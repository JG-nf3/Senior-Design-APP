from rest_framework import generics
from rest_framework.decorators import api_view
from rest_framework.response import Response
from .models import Update,Mailbox
from .serializers import MailboxSerializer, UpdateSerializer

@api_view()
def updateBox(request):
    return Response({'Status': 'success'})

class allMailboxes(generics.ListAPIView):
    queryset = Mailbox.objects.all()
    serializer_class = MailboxSerializer

@api_view(['POST'])
def addUpdate(request):
    serializer_class = UpdateSerializer(data=request.data)
    if serializer_class.is_valid():
        serializer_class.save()
    return Response(serializer_class.data)

@api_view(['GET'])
def userMailboxes(request, pk = None):
    queryset = Mailbox.objects.filter(owner = pk)
    serializer_class = MailboxSerializer(queryset, many=True)
    return Response(serializer_class.data)

@api_view(['GET'])
def userMailbox(request, pk = None):
    queryset = Mailbox.objects.get(id = pk)
    serializer_class = MailboxSerializer(queryset, many=False)
    return Response(serializer_class.data)

@api_view(['POST'])
def userUpdateMailbox(request, pk = None):
    queryset = Mailbox.objects.get(id = pk)
    serializer_class = MailboxSerializer(instance=queryset,data=request.data, many=False)
    if serializer_class.is_valid():
        serializer_class.save()
        return Response(serializer_class.data)
    return Response("Not valid")

@api_view(['DELETE'])
def userDeleteMailbox(request, pk=None):
    queryset = Mailbox.objects.get(id = pk)
    queryset.delete()

    return Response('Deleted')

