function v = helics_discard()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1329876557);
  end
  v = vInitialized;
end
