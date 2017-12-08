function v = helicsDiscard()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1);
  end
  v = vInitialized;
end
