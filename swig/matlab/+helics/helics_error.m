function v = helics_error()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876561);
  end
  v = vInitialized;
end
