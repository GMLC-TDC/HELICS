function v = helics_terminated()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876558);
  end
  v = vInitialized;
end
