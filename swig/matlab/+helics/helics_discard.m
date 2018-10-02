function v = helics_discard()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1432107591);
  end
  v = vInitialized;
end
