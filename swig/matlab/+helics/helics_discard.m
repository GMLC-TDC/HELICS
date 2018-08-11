function v = helics_discard()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 4);
  end
  v = vInitialized;
end
